#include <dobot_bringup/trajectory_executor.h>
#include <rclcpp/rclcpp.hpp>
#include <cmath>
#include <cstdio>

static const rclcpp::Logger kLogger = rclcpp::get_logger("trajectory_executor");

TrajectoryExecutor::TrajectoryExecutor(
    std::shared_ptr<CRCommanderRos2> commander,
    double servo_rate, double servo_t,
    double aheadtime, double gain)
    : commander_(commander),
      servo_rate_(servo_rate),
      servo_t_(servo_t),
      aheadtime_(aheadtime),
      gain_(gain)
{
}

TrajectoryExecutor::~TrajectoryExecutor()
{
    stop();
}

void TrajectoryExecutor::loadTrajectory(
    const trajectory_msgs::msg::JointTrajectory &traj)
{
    stop();

    std::lock_guard<std::mutex> lock(mutex_);
    timestamps_.clear();
    joints_deg_.clear();

    for (const auto &pt : traj.points)
    {
        double t = pt.time_from_start.sec + pt.time_from_start.nanosec / 1e9;
        timestamps_.push_back(t);

        std::array<double, 6> joints{};
        for (size_t i = 0; i < 6 && i < pt.positions.size(); i++)
        {
            joints[i] = rad2Deg(pt.positions[i]);
        }
        joints_deg_.push_back(joints);
    }

    state_ = State{};
    if (!timestamps_.empty())
    {
        state_.total_duration = timestamps_.back();
    }
}

void TrajectoryExecutor::start()
{
    if (running_)
        return;

    // Join a previously completed thread before creating a new one.
    // Without this, assigning to thread_ destroys the old joinable
    // std::thread, which calls std::terminate().
    if (thread_.joinable())
        thread_.join();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (timestamps_.empty())
        {
            state_.done = true;
            return;
        }
        state_.done = false;
        state_.error_msg.clear();
        state_.elapsed = 0.0;
    }

    running_ = true;
    thread_ = std::thread(&TrajectoryExecutor::servoLoop, this);
}

void TrajectoryExecutor::stop()
{
    if (!running_)
        return;

    running_ = false;

    if (thread_.joinable())
        thread_.join();

    // Flush the Dobot controller's internal command queue.
    if (commander_)
    {
        int32_t err_id = 0;
        commander_->sendServoCommand("Stop()", err_id);
    }
}

bool TrajectoryExecutor::isRunning() const
{
    return running_;
}

TrajectoryExecutor::State TrajectoryExecutor::getState() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return state_;
}

std::array<double, 6> TrajectoryExecutor::interpolateAt(double t) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return interpolate(t);
}

// MoveIt trajectories have waypoints at irregular time intervals.
// The servo loop sends commands at a fixed rate (every 20ms at 50Hz).
// Linearly interpolate joint positions (degrees) at time t between
// the two surrounding trajectory waypoints.  Caller must hold mutex_.
std::array<double, 6> TrajectoryExecutor::interpolate(double t) const
{
    // Caller must hold mutex_.
    if (joints_deg_.empty())
        return {};

    if (timestamps_.size() == 1 || t <= timestamps_.front())
        return joints_deg_.front();

    if (t >= timestamps_.back())
        return joints_deg_.back();

    // Find the segment containing t.
    size_t seg = 0;
    for (size_t i = 0; i < timestamps_.size() - 1; i++)
    {
        if (timestamps_[i + 1] >= t)
        {
            seg = i;
            break;
        }
    }

    double t0 = timestamps_[seg];
    double t1 = timestamps_[seg + 1];
    double alpha = (t1 > t0) ? (t - t0) / (t1 - t0) : 1.0;
    alpha = std::min(1.0, std::max(0.0, alpha));

    std::array<double, 6> result{};
    for (size_t j = 0; j < 6; j++)
    {
        result[j] = joints_deg_[seg][j] +
                     alpha * (joints_deg_[seg + 1][j] - joints_deg_[seg][j]);
    }
    return result;
}

void TrajectoryExecutor::servoLoop()
{
    RCLCPP_INFO(kLogger,
                "Servo loop started: rate=%.1f Hz, t=%.4f s, "
                "aheadtime=%.1f, gain=%.1f, points=%zu",
                servo_rate_, servo_t_, aheadtime_, gain_,
                joints_deg_.size());

    // Wait for robot to be in ENABLE mode (5) before streaming ServoJ.
    // ServoJ is rejected if the robot is still RUNNING (7) from a
    // previous command.
    auto wait_start = std::chrono::steady_clock::now();
    while (running_)
    {
        RealTimeData rd = commander_->getRealData();
        uint64_t mode = rd.robot_mode;
        if (mode == 5)
            break;
        if (mode == 9 || mode == 11)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                "Robot in error mode " + std::to_string(mode) +
                " before trajectory start";
            state_.done = true;
            RCLCPP_ERROR(kLogger, "Robot mode %lu before start — aborting",
                         static_cast<unsigned long>(mode));
            running_ = false;
            return;
        }
        if (std::chrono::steady_clock::now() - wait_start >
            std::chrono::seconds(10))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                "Timeout waiting for ENABLE mode (current: " +
                std::to_string(mode) + ")";
            state_.done = true;
            RCLCPP_ERROR(kLogger, "Timeout waiting for robot ENABLE mode");
            running_ = false;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    auto start_time = std::chrono::steady_clock::now();
    const auto period = std::chrono::duration_cast<
        std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(1.0 / servo_rate_));

    double total_duration;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        total_duration = state_.total_duration;
    }

    uint64_t iteration = 0;

    while (running_)
    {
        auto now = std::chrono::steady_clock::now();
        double elapsed =
            std::chrono::duration<double>(now - start_time).count();

        // Trajectory complete?
        if (elapsed >= total_duration)
        {
            // Send the final point one more time to ensure convergence.
            std::array<double, 6> final_target;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                final_target = joints_deg_.back();
            }

            char cmd[256];
            std::snprintf(cmd, sizeof(cmd),
                          "ServoJ(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
                          "t=%.4f,aheadtime=%.1f,gain=%.1f)",
                          final_target[0], final_target[1], final_target[2],
                          final_target[3], final_target[4], final_target[5],
                          servo_t_, aheadtime_, gain_);

            int32_t err_id = 0;
            commander_->sendServoCommand(std::string(cmd), err_id);

            std::lock_guard<std::mutex> lock(mutex_);
            state_.elapsed = total_duration;
            state_.desired = final_target;
            state_.done = true;
            RCLCPP_INFO(kLogger, "Trajectory complete: %.2f s", elapsed);
            break;
        }

        // Interpolate target joints at current time.
        std::array<double, 6> target;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            target = interpolate(elapsed);
        }

        // Check robot mode from real-time data.
        RealTimeData real_data = commander_->getRealData();
        uint64_t mode = real_data.robot_mode;
        if (mode == 9 || mode == 11)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                "Robot entered error mode " + std::to_string(mode);
            state_.done = true;
            RCLCPP_ERROR(kLogger,
                         "Robot mode %lu — aborting trajectory",
                         static_cast<unsigned long>(mode));
            break;
        }

        // Format and send ServoJ command.
        char cmd[256];
        std::snprintf(cmd, sizeof(cmd),
                      "ServoJ(%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,"
                      "t=%.4f,aheadtime=%.1f,gain=%.1f)",
                      target[0], target[1], target[2],
                      target[3], target[4], target[5],
                      servo_t_, aheadtime_, gain_);

        int32_t err_id = 0;
        bool ok = commander_->sendServoCommand(std::string(cmd), err_id);

        if (!ok || err_id != 0)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                "ServoJ failed: err_id=" + std::to_string(err_id);
            state_.done = true;
            RCLCPP_ERROR(kLogger, "ServoJ failed: err_id=%d", err_id);
            break;
        }

        // Update state.
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.elapsed = elapsed;
            state_.desired = target;
            for (size_t i = 0; i < 6; i++)
            {
                state_.actual[i] = real_data.q_actual[i];
                state_.error[i] = target[i] - real_data.q_actual[i];
            }
        }

        // Drift-compensated sleep: target next_time relative to start,
        // not relative to "now", to prevent accumulated drift.
        iteration++;
        auto next_time = start_time +
                         static_cast<long long>(iteration) * period;
        std::this_thread::sleep_until(next_time);
    }

    running_ = false;
}

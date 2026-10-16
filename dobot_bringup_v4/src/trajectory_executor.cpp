#include <dobot_bringup/trajectory_executor.h>
#include <rclcpp/rclcpp.hpp>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <string>

#include <pthread.h>
#include <sched.h>

static const rclcpp::Logger kLogger = rclcpp::get_logger("trajectory_executor");
// The controller pushes a realtime packet every ~8 ms. Half a second is 60
// missed packets — comfortably past any jitter, and short of the ~3 s a
// reconnect takes, so a genuine blip still gets the full 10 s wait below.
static constexpr int64_t kRealtimeStaleMs = 500;

namespace
{
// One sent ServoJ, kept so a controller rejection can be traced to the
// commands just before it.
struct SentServo
{
    double elapsed;                 // trajectory time of the target (s)
    double gap_ms;                  // wall time since the previous send
    std::array<double, 6> target;   // deg
    std::array<double, 6> actual;   // deg, controller feedback at send time
    std::array<double, 6> ctrl_target;  // deg, the controller's own target then
    double tcp_target;              // mm/s, the controller's target tool speed
    double tcp_actual;              // mm/s, its measured tool speed
};

// The linear part of the feed's TCP speed (x, y, z; mm/s).
double linearSpeed(const double *v)
{
    return std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

constexpr size_t kRecentSends = 8;
// The controller's target further than this from the arm before a stream is
// worth a warning.
constexpr double kTargetOffsetWarnDeg = 0.5;

std::string joints(const std::array<double, 6> &v, const char *fmt = "%.3f")
{
    std::string out = "[";
    char buf[32];
    for (size_t j = 0; j < 6; j++)
    {
        std::snprintf(buf, sizeof(buf), fmt, v[j]);
        out += buf;
        if (j < 5)
            out += ", ";
    }
    return out + "]";
}

// The commands before a failure: target, actual, and the joint speed the
// controller derives from consecutive targets (step / t).
void logRecentSends(const std::deque<SentServo> &sends, double servo_t)
{
    RCLCPP_ERROR(kLogger, "Last %zu ServoJ commands (deg; speed = step / t=%.3f s):",
                 sends.size(), servo_t);
    for (size_t i = 0; i < sends.size(); i++)
    {
        const SentServo &s = sends[i];
        std::array<double, 6> speed{};
        if (i > 0)
            for (size_t j = 0; j < 6; j++)
                speed[j] = (s.target[j] - sends[i - 1].target[j]) / servo_t;
        RCLCPP_ERROR(kLogger, "  t=%.3f gap=%.1f ms target=%s actual=%s speed=%s deg/s "
                     "controller target=%s tool speed target %.0f actual %.0f mm/s",
                     s.elapsed, s.gap_ms, joints(s.target).c_str(),
                     joints(s.actual).c_str(), joints(speed, "%.1f").c_str(),
                     joints(s.ctrl_target).c_str(), s.tcp_target, s.tcp_actual);
    }
}
}  // namespace

TrajectoryExecutor::TrajectoryExecutor(
    std::shared_ptr<CRCommanderRos2> commander,
    double servo_rate, double servo_t,
    double aheadtime, double gain, int rt_priority)
    : commander_(commander),
      servo_rate_(servo_rate),
      servo_t_(servo_t),
      aheadtime_(aheadtime),
      gain_(gain),
      rt_priority_(rt_priority)
{
}

double TrajectoryExecutor::nextDue(double due, double sent, double period)
{
    return (sent - due > period / 2 ? sent : due) + period;
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
    // A late wake-up delays a ServoJ; the controller has nothing new to aim
    // at meanwhile and the arm dips. Ahead of the camera and planning threads
    // on a busy Pi, the servo thread wakes on time. Needs RLIMIT_RTPRIO
    // (LimitRTPRIO= in the systemd unit) or CAP_SYS_NICE.
    if (rt_priority_ > 0)
    {
        sched_param sp{};
        sp.sched_priority = rt_priority_;
        int err = pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp);
        static std::atomic<bool> warned{false};
        if (err != 0 && !warned.exchange(true))
            RCLCPP_WARN(kLogger, "Servo thread stays at normal priority: SCHED_FIFO %d refused (%s)",
                        rt_priority_, std::strerror(err));
    }

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
        // robot_mode comes off the realtime feed (TCP 30004), not the
        // dashboard. When that feed stalls the struct keeps whatever it last
        // held, so "the robot will not enable" and "we have no idea what the
        // robot is doing" arrive here looking identical — and the operator
        // chasing the resulting "current: 0" goes and stares at the arm
        // instead of the link. Tell them apart.
        int64_t age_ms = commander_->realtimeAgeMs();
        bool stale = age_ms < 0 || age_ms > kRealtimeStaleMs;

        uint64_t mode = 0;
        if (!stale)
        {
            RealTimeData rd = commander_->getRealData();
            mode = rd.robot_mode;
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
        }
        if (std::chrono::steady_clock::now() - wait_start >
            std::chrono::seconds(10))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                stale ? "Realtime feedback stale (" +
                            (age_ms < 0 ? std::string("no packet ever received")
                                        : std::to_string(age_ms) + " ms since the last one") +
                            ") — robot mode unknown; check the TCP 30004 link"
                      : "Timeout waiting for ENABLE mode (current: " +
                            std::to_string(mode) + ")";
            state_.done = true;
            RCLCPP_ERROR(kLogger, "%s", state_.error_msg.c_str());
            running_ = false;
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Where the controller is aiming before the first command. A target away
    // from the arm means it still holds an earlier motion's, and streaming
    // may start by driving towards it.
    {
        RealTimeData rd = commander_->getRealData();
        std::array<double, 6> ctrl{}, actual{};
        double worst = 0.0;
        for (size_t j = 0; j < 6; j++)
        {
            ctrl[j] = rd.q_target[j];
            actual[j] = rd.q_actual[j];
            worst = std::max(worst, std::fabs(ctrl[j] - actual[j]));
        }
        if (worst > kTargetOffsetWarnDeg)
            RCLCPP_WARN(kLogger, "Before streaming: the controller's target %s is %.2f deg "
                        "from the arm %s", joints(ctrl).c_str(), worst, joints(actual).c_str());
        else
            RCLCPP_INFO(kLogger, "Before streaming: the controller's target is on the arm "
                        "(within %.3f deg)", worst);
    }

    auto start_time = std::chrono::steady_clock::now();
    const double period = 1.0 / servo_rate_;
    auto since_start = [&start_time](std::chrono::steady_clock::time_point t) {
        return std::chrono::duration<double>(t - start_time).count();
    };

    double total_duration;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        total_duration = state_.total_duration;
    }

    uint64_t iteration = 0;
    std::deque<SentServo> recent;
    auto last_send = start_time;
    double max_gap_ms = 0.0;
    double due = 0.0;               // when this command was due (s from start)
    int late_sends = 0;
    double max_wake_late_ms = 0.0;  // how late the thread woke for a command
    double max_reply_ms = 0.0;      // the controller's slowest ServoJ reply

    while (running_)
    {
        // The trajectory point for this command's slot, whenever it is sent.
        double elapsed = static_cast<double>(iteration) * period;

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
            RCLCPP_INFO(kLogger, "Trajectory complete: %.2f s (planned %.2f s), longest gap "
                        "between ServoJ commands %.1f ms (nominal %.1f ms), %d sent more "
                        "than half a period late; latest wake-up %.1f ms, slowest reply %.1f ms",
                        since_start(std::chrono::steady_clock::now()), total_duration,
                        max_gap_ms, 1000.0 / servo_rate_, late_sends, max_wake_late_ms,
                        max_reply_ms);
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
            logRecentSends(recent, servo_t_);
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
        auto send_time = std::chrono::steady_clock::now();
        bool ok = commander_->sendServoCommand(std::string(cmd), err_id);
        auto reply_time = std::chrono::steady_clock::now();
        if (iteration > 0)
            max_wake_late_ms = std::max(max_wake_late_ms, (since_start(send_time) - due) * 1e3);
        max_reply_ms = std::max(
            max_reply_ms, std::chrono::duration<double, std::milli>(reply_time - send_time).count());

        SentServo sent{elapsed,
                       std::chrono::duration<double, std::milli>(send_time - last_send).count(),
                       target, {}, {}, linearSpeed(real_data.TCP_speed_target),
                       linearSpeed(real_data.TCP_speed_actual)};
        for (size_t j = 0; j < 6; j++)
        {
            sent.actual[j] = real_data.q_actual[j];
            sent.ctrl_target[j] = real_data.q_target[j];
        }
        if (iteration > 0)
            max_gap_ms = std::max(max_gap_ms, sent.gap_ms);
        last_send = send_time;
        recent.push_back(sent);
        if (recent.size() > kRecentSends)
            recent.pop_front();

        if (!ok || err_id != 0)
        {
            std::lock_guard<std::mutex> lock(mutex_);
            state_.error_msg =
                "ServoJ failed: err_id=" + std::to_string(err_id);
            state_.done = true;
            RCLCPP_ERROR(kLogger, "ServoJ failed: err_id=%d", err_id);
            logRecentSends(recent, servo_t_);
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

        iteration++;
        double sent_at = since_start(send_time);
        if (sent_at - due > period / 2)
            late_sends++;
        due = nextDue(due, sent_at, period);
        std::this_thread::sleep_until(
            start_time + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                             std::chrono::duration<double>(due)));
    }

    running_ = false;
}

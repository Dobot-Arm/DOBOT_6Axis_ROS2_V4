/**
 * TrajectoryExecutor — streams ServoJ commands from a JointTrajectory.
 *
 * Resamples a MoveIt-planned trajectory to a fixed rate and sends ServoJ
 * commands directly over the dashboard TCP, bypassing the ROS service layer
 * for minimal latency.  Robot mode is monitored from the real-time feedback
 * channel; the loop aborts on ERROR (9) or COLLISION (11).
 */

#ifndef TRAJECTORY_EXECUTOR_H
#define TRAJECTORY_EXECUTOR_H

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <trajectory_msgs/msg/joint_trajectory.hpp>
#include <dobot_bringup/command.h>

class TrajectoryExecutor
{
public:
    TrajectoryExecutor(std::shared_ptr<CRCommanderRos2> commander,
                       double servo_rate = 50.0,
                       double servo_t = 0.02,
                       double aheadtime = 50.0,
                       double gain = 500.0,
                       int rt_priority = 0);

    ~TrajectoryExecutor();

    // Load a new trajectory.  Stops current execution if running.
    // Joint positions must be in radians (ROS convention); they are
    // converted to degrees for the Dobot controller internally.
    void loadTrajectory(const trajectory_msgs::msg::JointTrajectory &traj);

    // Start streaming ServoJ commands.
    void start();

    // Stop streaming and send Dobot Stop() to flush the controller queue.
    void stop();

    bool isRunning() const;

    struct State
    {
        double elapsed = 0.0;
        double total_duration = 0.0;
        std::array<double, 6> desired{};  // current target (degrees)
        std::array<double, 6> actual{};   // from real-time feedback (degrees)
        std::array<double, 6> error{};    // desired - actual (degrees)
        bool done = false;
        std::string error_msg;            // non-empty on failure
    };

    State getState() const;

    // Query the interpolated joint position (degrees) at time t.
    // Thread-safe — acquires mutex internally.  Useful for testing
    // and for external code that needs to preview the trajectory.
    std::array<double, 6> interpolateAt(double t) const;

    static constexpr double kPi = 3.14159265358979323846;
    static inline double rad2Deg(double rad) { return rad * 180.0 / kPi; }
    static inline double deg2Rad(double deg) { return deg * kPi / 180.0; }

    // When the next ServoJ is due (s from the stream's start), given when the
    // last one was due and when it was sent: one period later, or one period
    // after the send if that was more than kLateSend late. Each command
    // carries the trajectory point for its own slot (index × period), so a
    // late send delays the rest of the stream instead of being made up. The
    // next command never follows a late one early: on the CR5, a ServoJ 10 ms
    // early made the controller's target speed surge 30 % and raised alarm 225.
    static double nextDue(double due, double sent, double period);

    // How late a send may be and still keep the schedule (s): wake-up jitter.
    static constexpr double kLateSend = 0.001;

private:
    void servoLoop();
    std::array<double, 6> interpolate(double t) const;  // caller must hold mutex_

    std::shared_ptr<CRCommanderRos2> commander_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    mutable std::mutex mutex_;  // guards trajectory data and state_

    // Trajectory data — stored in degrees (Dobot convention)
    std::vector<double> timestamps_;
    std::vector<std::array<double, 6>> joints_deg_;

    // Configuration
    double servo_rate_;
    double servo_t_;
    double aheadtime_;
    double gain_;
    int rt_priority_;               // SCHED_FIFO priority of the servo thread; 0: none

    // Current execution state
    State state_;
};

#endif  // TRAJECTORY_EXECUTOR_H

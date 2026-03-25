/**
 * Unit tests for TrajectoryExecutor.
 *
 * These tests exercise interpolation, rad/deg conversion, trajectory loading,
 * and edge cases.  They do NOT require a robot connection — the executor is
 * constructed with a nullptr commander and never started.
 */

#include <gtest/gtest.h>
#include <dobot_bringup/trajectory_executor.h>
#include <trajectory_msgs/msg/joint_trajectory.hpp>
#include <cmath>

using trajectory_msgs::msg::JointTrajectory;
using trajectory_msgs::msg::JointTrajectoryPoint;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static constexpr double kDeg2Rad = M_PI / 180.0;
static constexpr double kRad2Deg = 180.0 / M_PI;
static constexpr double kTol = 1e-6;  // tolerance for floating-point comparison

// Build a JointTrajectoryPoint with 6 joint positions (radians) and a timestamp.
static JointTrajectoryPoint makePoint(
    std::array<double, 6> joints_rad, double time_sec)
{
    JointTrajectoryPoint pt;
    for (auto j : joints_rad)
        pt.positions.push_back(j);
    pt.time_from_start.sec = static_cast<int32_t>(time_sec);
    pt.time_from_start.nanosec =
        static_cast<uint32_t>((time_sec - std::floor(time_sec)) * 1e9);
    return pt;
}

// Build a simple trajectory with the given waypoints.
static JointTrajectory makeTrajectory(
    const std::vector<std::pair<std::array<double, 6>, double>> &waypoints)
{
    JointTrajectory traj;
    traj.joint_names = {"joint1", "joint2", "joint3",
                        "joint4", "joint5", "joint6"};
    for (const auto &wp : waypoints)
        traj.points.push_back(makePoint(wp.first, wp.second));
    return traj;
}

// Create a TrajectoryExecutor with no commander (for interpolation tests only).
static std::unique_ptr<TrajectoryExecutor> makeExecutor()
{
    return std::make_unique<TrajectoryExecutor>(nullptr);
}

// ---------------------------------------------------------------------------
// rad2Deg / deg2Rad
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorConversion, Rad2Deg)
{
    EXPECT_NEAR(TrajectoryExecutor::rad2Deg(0.0), 0.0, kTol);
    EXPECT_NEAR(TrajectoryExecutor::rad2Deg(M_PI), 180.0, kTol);
    EXPECT_NEAR(TrajectoryExecutor::rad2Deg(-M_PI / 2), -90.0, kTol);
    EXPECT_NEAR(TrajectoryExecutor::rad2Deg(2 * M_PI), 360.0, kTol);
}

TEST(TrajectoryExecutorConversion, Deg2Rad)
{
    EXPECT_NEAR(TrajectoryExecutor::deg2Rad(0.0), 0.0, kTol);
    EXPECT_NEAR(TrajectoryExecutor::deg2Rad(180.0), M_PI, kTol);
    EXPECT_NEAR(TrajectoryExecutor::deg2Rad(-90.0), -M_PI / 2, kTol);
}

TEST(TrajectoryExecutorConversion, Roundtrip)
{
    for (double deg = -360.0; deg <= 360.0; deg += 17.3)
    {
        double result = TrajectoryExecutor::rad2Deg(
            TrajectoryExecutor::deg2Rad(deg));
        EXPECT_NEAR(result, deg, kTol) << "roundtrip failed for " << deg;
    }
}

// ---------------------------------------------------------------------------
// loadTrajectory + getState
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorLoad, EmptyTrajectory)
{
    auto exec = makeExecutor();
    JointTrajectory traj;
    exec->loadTrajectory(traj);

    auto state = exec->getState();
    EXPECT_EQ(state.total_duration, 0.0);
    EXPECT_FALSE(exec->isRunning());
}

TEST(TrajectoryExecutorLoad, SinglePoint)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{10 * kDeg2Rad, 20 * kDeg2Rad, 30 * kDeg2Rad,
          40 * kDeg2Rad, 50 * kDeg2Rad, 60 * kDeg2Rad},
         0.0},
    });
    exec->loadTrajectory(traj);

    auto state = exec->getState();
    EXPECT_NEAR(state.total_duration, 0.0, kTol);
}

TEST(TrajectoryExecutorLoad, DurationFromLastPoint)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{0, 0, 0, 0, 0, 0}, 1.5},
        {{0, 0, 0, 0, 0, 0}, 3.7},
    });
    exec->loadTrajectory(traj);

    auto state = exec->getState();
    EXPECT_NEAR(state.total_duration, 3.7, kTol);
}

TEST(TrajectoryExecutorLoad, RadiansConvertedToDegrees)
{
    auto exec = makeExecutor();
    // Load a single point at pi/2 radians (= 90 degrees)
    auto traj = makeTrajectory({
        {{M_PI / 2, -M_PI / 4, M_PI, 0, -M_PI / 6, M_PI / 3}, 0.0},
    });
    exec->loadTrajectory(traj);

    // interpolateAt should return degrees
    auto joints = exec->interpolateAt(0.0);
    EXPECT_NEAR(joints[0], 90.0, kTol);
    EXPECT_NEAR(joints[1], -45.0, kTol);
    EXPECT_NEAR(joints[2], 180.0, kTol);
    EXPECT_NEAR(joints[3], 0.0, kTol);
    EXPECT_NEAR(joints[4], -30.0, kTol);
    EXPECT_NEAR(joints[5], 60.0, kTol);
}

TEST(TrajectoryExecutorLoad, StartNotDone)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{1, 1, 1, 1, 1, 1}, 2.0},
    });
    exec->loadTrajectory(traj);

    auto state = exec->getState();
    EXPECT_FALSE(state.done);
    EXPECT_TRUE(state.error_msg.empty());
}

TEST(TrajectoryExecutorLoad, ReloadClearsState)
{
    auto exec = makeExecutor();

    // Load first trajectory
    auto traj1 = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{1, 1, 1, 1, 1, 1}, 5.0},
    });
    exec->loadTrajectory(traj1);
    EXPECT_NEAR(exec->getState().total_duration, 5.0, kTol);

    // Load second trajectory — should replace first
    auto traj2 = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{2, 2, 2, 2, 2, 2}, 2.0},
    });
    exec->loadTrajectory(traj2);
    EXPECT_NEAR(exec->getState().total_duration, 2.0, kTol);
}

// ---------------------------------------------------------------------------
// Interpolation
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorInterp, EmptyReturnsZeros)
{
    auto exec = makeExecutor();
    JointTrajectory traj;
    exec->loadTrajectory(traj);

    auto joints = exec->interpolateAt(0.0);
    for (size_t i = 0; i < 6; i++)
        EXPECT_EQ(joints[i], 0.0);
}

TEST(TrajectoryExecutorInterp, SinglePointAlwaysReturnsSame)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{45 * kDeg2Rad, 0, 0, 0, 0, 0}, 0.0},
    });
    exec->loadTrajectory(traj);

    // Before, at, and after the point — should all return same value
    for (double t : {-1.0, 0.0, 0.5, 100.0})
    {
        auto joints = exec->interpolateAt(t);
        EXPECT_NEAR(joints[0], 45.0, kTol) << "at t=" << t;
    }
}

TEST(TrajectoryExecutorInterp, TwoPointLinear)
{
    auto exec = makeExecutor();
    // From 0 deg to 90 deg over 2 seconds
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{90 * kDeg2Rad, 0, 0, 0, 0, 0}, 2.0},
    });
    exec->loadTrajectory(traj);

    // At start
    EXPECT_NEAR(exec->interpolateAt(0.0)[0], 0.0, kTol);

    // At end
    EXPECT_NEAR(exec->interpolateAt(2.0)[0], 90.0, kTol);

    // At midpoint
    EXPECT_NEAR(exec->interpolateAt(1.0)[0], 45.0, kTol);

    // At quarter
    EXPECT_NEAR(exec->interpolateAt(0.5)[0], 22.5, kTol);

    // At three-quarters
    EXPECT_NEAR(exec->interpolateAt(1.5)[0], 67.5, kTol);
}

TEST(TrajectoryExecutorInterp, ThreePointSegments)
{
    auto exec = makeExecutor();
    // Segment 1: 0→60 deg over 0–1s
    // Segment 2: 60→30 deg over 1–3s
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{60 * kDeg2Rad, 0, 0, 0, 0, 0}, 1.0},
        {{30 * kDeg2Rad, 0, 0, 0, 0, 0}, 3.0},
    });
    exec->loadTrajectory(traj);

    // Segment 1 midpoint (t=0.5): expect 30 deg
    EXPECT_NEAR(exec->interpolateAt(0.5)[0], 30.0, kTol);

    // Boundary (t=1.0): expect 60 deg
    EXPECT_NEAR(exec->interpolateAt(1.0)[0], 60.0, kTol);

    // Segment 2 midpoint (t=2.0): expect 45 deg
    EXPECT_NEAR(exec->interpolateAt(2.0)[0], 45.0, kTol);

    // End (t=3.0): expect 30 deg
    EXPECT_NEAR(exec->interpolateAt(3.0)[0], 30.0, kTol);
}

TEST(TrajectoryExecutorInterp, AllJointsInterpolated)
{
    auto exec = makeExecutor();
    std::array<double, 6> start_deg = {10, 20, 30, 40, 50, 60};
    std::array<double, 6> end_deg = {110, 120, 130, 140, 150, 160};

    std::array<double, 6> start_rad, end_rad;
    for (size_t i = 0; i < 6; i++)
    {
        start_rad[i] = start_deg[i] * kDeg2Rad;
        end_rad[i] = end_deg[i] * kDeg2Rad;
    }

    auto traj = makeTrajectory({
        {start_rad, 0.0},
        {end_rad, 4.0},
    });
    exec->loadTrajectory(traj);

    // At midpoint (t=2.0), each joint should be halfway
    auto joints = exec->interpolateAt(2.0);
    for (size_t i = 0; i < 6; i++)
    {
        double expected = (start_deg[i] + end_deg[i]) / 2.0;
        EXPECT_NEAR(joints[i], expected, kTol) << "joint " << i;
    }
}

TEST(TrajectoryExecutorInterp, NegativeAngles)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{-90 * kDeg2Rad, 0, 0, 0, 0, 0}, 0.0},
        {{90 * kDeg2Rad, 0, 0, 0, 0, 0}, 2.0},
    });
    exec->loadTrajectory(traj);

    // Midpoint: should be 0 degrees
    EXPECT_NEAR(exec->interpolateAt(1.0)[0], 0.0, kTol);

    // Quarter: should be -45 degrees
    EXPECT_NEAR(exec->interpolateAt(0.5)[0], -45.0, kTol);
}

// ---------------------------------------------------------------------------
// Clamping: t before start / after end
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorInterp, BeforeStartClampsToFirst)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{45 * kDeg2Rad, 0, 0, 0, 0, 0}, 0.5},
        {{90 * kDeg2Rad, 0, 0, 0, 0, 0}, 2.0},
    });
    exec->loadTrajectory(traj);

    // t=0 is before the first timestamp (0.5) — should return first point
    auto joints = exec->interpolateAt(0.0);
    EXPECT_NEAR(joints[0], 45.0, kTol);
}

TEST(TrajectoryExecutorInterp, AfterEndClampsToLast)
{
    auto exec = makeExecutor();
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{90 * kDeg2Rad, 0, 0, 0, 0, 0}, 2.0},
    });
    exec->loadTrajectory(traj);

    // t=100 is way past the end — should return last point
    auto joints = exec->interpolateAt(100.0);
    EXPECT_NEAR(joints[0], 90.0, kTol);
}

// ---------------------------------------------------------------------------
// Lifecycle (no commander — just state machine checks)
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorLifecycle, NotRunningByDefault)
{
    auto exec = makeExecutor();
    EXPECT_FALSE(exec->isRunning());
}

TEST(TrajectoryExecutorLifecycle, StartEmptyTrajectoryMarksDone)
{
    auto exec = makeExecutor();
    JointTrajectory traj;
    exec->loadTrajectory(traj);
    exec->start();

    // Empty trajectory → done immediately, never actually running
    EXPECT_FALSE(exec->isRunning());
    EXPECT_TRUE(exec->getState().done);
}

TEST(TrajectoryExecutorLifecycle, SubsecondTimestamp)
{
    auto exec = makeExecutor();
    // Test that fractional seconds are parsed correctly
    auto traj = makeTrajectory({
        {{0, 0, 0, 0, 0, 0}, 0.0},
        {{M_PI, 0, 0, 0, 0, 0}, 0.456},
    });
    exec->loadTrajectory(traj);

    EXPECT_NEAR(exec->getState().total_duration, 0.456, 1e-3);
}

// ---------------------------------------------------------------------------
// Many-segment trajectory (stress test for segment finding)
// ---------------------------------------------------------------------------

TEST(TrajectoryExecutorInterp, ManySegments)
{
    auto exec = makeExecutor();
    // 100 points, joint0 goes from 0 to 99 degrees linearly
    std::vector<std::pair<std::array<double, 6>, double>> waypoints;
    for (int i = 0; i < 100; i++)
    {
        std::array<double, 6> joints{};
        joints[0] = static_cast<double>(i) * kDeg2Rad;
        waypoints.push_back({joints, static_cast<double>(i) * 0.1});
    }
    exec->loadTrajectory(makeTrajectory(waypoints));

    // Check a few points
    EXPECT_NEAR(exec->interpolateAt(0.0)[0], 0.0, kTol);
    EXPECT_NEAR(exec->interpolateAt(4.95)[0], 49.5, 0.01);
    EXPECT_NEAR(exec->interpolateAt(9.9)[0], 99.0, kTol);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

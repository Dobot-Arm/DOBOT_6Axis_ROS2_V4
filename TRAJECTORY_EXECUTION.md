# Trajectory Execution via ServoJ Streaming

## Problem

The Dobot CR5 driver exposes every dashboard command as a ROS service but has no trajectory execution capability.
A possible workaround — planning in MoveIt with `plan_only=True` then sending individual MovJ commands — causes the robot to cut corners because the Dobot controller plans its own interpolation between waypoints, ignoring the collision-free path MoveIt computed.

## Solution

A C++ `TrajectoryExecutor` class added directly to the `dobot_bringup_v4` node. It streams ServoJ commands over the dashboard TCP at a fixed rate, following the MoveIt-planned path faithfully.

## What Changed

### New files

| File | Purpose |
|---|---|
| `dobot_bringup_v4/include/dobot_bringup/trajectory_executor.h` | TrajectoryExecutor class declaration |
| `dobot_bringup_v4/src/trajectory_executor.cpp` | Servo loop, interpolation, error handling |

### Modified files

| File | Change |
|---|---|
| `dobot_bringup_v4/include/dobot_bringup/command.h` | Added `sendServoCommand()` (low-overhead TCP send) and `dash_mutex_` for thread safety |
| `dobot_bringup_v4/src/command.cpp` | Implemented `sendServoCommand()`. Added `dash_mutex_` locking to all `callRosService` overloads |
| `dobot_bringup_v4/include/dobot_bringup/cr_robot_ros2.h` | Added FollowJointTrajectory action server, TrajectoryExecutor member, action callbacks |
| `dobot_bringup_v4/src/cr_robot_ros2.cpp` | Servo parameters, executor init, action server creation, goal/cancel/execute callbacks |
| `dobot_bringup_v4/CMakeLists.txt` | Added `trajectory_executor.cpp` source, `control_msgs` and `trajectory_msgs` dependencies |
| `dobot_bringup_v4/package.xml` | Added `control_msgs`, `trajectory_msgs`, `sensor_msgs` dependencies |

## Architecture

```
MoveIt (plan_only)
    │
    ▼ JointTrajectory (radians, variable-time waypoints)
┌──────────────────────────────────────────────────┐
│ CRRobotRos2 node                                 │
│                                                  │
│  FollowJointTrajectory action server             │
│    ├─ handleGoalFJT()    → accept                │
│    ├─ handleCancelFJT()  → stop executor         │
│    └─ executeTrajectory() (detached thread)       │
│         │                                        │
│         ▼                                        │
│  TrajectoryExecutor                              │
│    ├─ loadTrajectory()  rad→deg, store waypoints │
│    ├─ start()           spawn servoLoop thread   │
│    ├─ servoLoop()                                │
│    │    ├─ interpolate(t) → target joints (deg)  │
│    │    ├─ check robot_mode from real-time data  │
│    │    ├─ sendServoCommand() → dashboard TCP    │
│    │    └─ drift-compensated sleep               │
│    └─ stop()            send Stop(), join thread │
│                                                  │
│  Existing services (MovJ, MovL, etc.)            │
│    └─ callRosService() → dashboard TCP (locked)  │
└──────────────────────────────────────────────────┘
         │                          │
    port 29999                 port 30004
   (dashboard TCP)          (real-time feedback)
         │                          │
         ▼                          ▼
       Dobot CR5 Controller
```

## How It Works

1. MoveIt plans a trajectory with variable-time waypoints in joint space (radians).
2. The `FollowJointTrajectory` action server receives the trajectory.
3. `TrajectoryExecutor::loadTrajectory()` converts radians to degrees and stores the waypoints.
4. `servoLoop()` runs in a dedicated thread:
   - Reads wall-clock elapsed time since start.
   - Linearly interpolates the target joint positions at the current time.
   - Checks robot mode from the 125 Hz real-time feedback channel — aborts on ERROR (9) or COLLISION (11).
   - Formats a `ServoJ(j1,j2,...,j6,t=T,aheadtime=A,gain=G)` command.
   - Sends it via `sendServoCommand()` — direct TCP with minimal overhead, no per-call logging.
   - Sleeps until `start_time + iteration * period` (drift-compensated).
5. The action server publishes feedback at 20 Hz (desired/actual/error in radians) and reports success or abort when the loop finishes.

## Thread Safety

A `dash_mutex_` in `CRCommanderRos2` serialises all dashboard TCP access. Both `callRosService()` (used by ROS service handlers) and `sendServoCommand()` (used by the servo thread) acquire this mutex. This means:

- Service calls during trajectory execution will briefly block the servo loop (and vice versa).
- For smooth execution, avoid calling dashboard services while a trajectory is running.
- The real-time feedback channel (port 30004) is read-only and unaffected.

## ROS Parameters

Added to the `dobot_bringup_ros2` node:

| Parameter | Default | Description |
|---|---|---|
| `servo_rate` | 50.0 | ServoJ streaming rate (Hz). The ServoJ time parameter (`t`) is derived as 1/servo_rate. |
| `servo_aheadtime` | 50.0 | Controller lookahead / D-term [20..100] |
| `servo_gain` | 500.0 | Controller proportional gain [200..1000] |

## Action Server Interface

**Topic:** `/{robot_type}_group_controller/follow_joint_trajectory`

Where `{robot_type}` comes from the `robot_type` parameter (default: `cr5`).

**Type:** `control_msgs/action/FollowJointTrajectory`

- **Goal:** `trajectory` field (JointTrajectory with positions in radians)
- **Feedback:** `desired`, `actual`, `error` (TrajectoryPoint, radians) at 20 Hz
- **Result:** `SUCCESSFUL` (0) or `PATH_TOLERANCE_VIOLATED` (-4) with error string

## Tuning

The ServoJ parameters control how the Dobot's internal controller tracks the commanded positions:

- **`servo_t`** — Time interval between commands. Must match `1/servo_rate`. Default 0.02s = 50 Hz.
- **`servo_aheadtime`** — Lookahead time (derivative-like). Higher = smoother but more lag. Range: 20–100.
- **`servo_gain`** — Proportional gain. Higher = stiffer tracking but can oscillate. Range: 200–1000.

Start with defaults. If the robot oscillates or is jerky, reduce `servo_gain`. If it lags behind the trajectory, increase `servo_aheadtime`.

## What Still Works

All existing functionality is preserved:

- ROS services (MovJ, MovL, ServoJ, EnableRobot, ClearError, Stop, etc.)
- Joint state publishing
- Robot status publishing
- Real-time feedback topic

The `execute_movj()` method in drover's `arm_control.py` still works for blended approach+touch sequences where CP blending with follow-up MovL commands is needed.

## Dependencies

Packages required (install if missing):

```
sudo apt install ros-humble-control-msgs ros-humble-trajectory-msgs
```

## Future

The `TrajectoryExecutor` design supports a future `updateTrajectory()` method that swaps the trajectory buffer under the mutex, enabling real-time path adjustment for wind compensation without stopping the servo loop.

Migration to `ros2_control` (a proper `SystemInterface` hardware plugin) is possible later — the servo loop translates directly into a `write()` method, and the real-time feedback reading into `read()`.

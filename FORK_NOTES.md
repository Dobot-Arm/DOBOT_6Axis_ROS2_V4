# Fork Notes

This fork adapts the upstream Dobot 6-Axis ROS 2 V4 driver for use with
MoveIt 2, with a focus on the CR5 arm. The upstream README still applies
for installation, dependencies, and TCP/IP protocol details — this file
documents only what diverges from upstream.

## Goal

Drive the arm along MoveIt-planned trajectories with fidelity to the
planned path, rather than letting the on-board controller re-interpolate
between waypoints and cut corners. This requires:

- a `FollowJointTrajectory` action interface that MoveIt can target
  directly,
- streaming joint commands at a fixed rate via the controller's ServoJ
  primitive instead of the higher-level MovJ/MovL motion primitives,
- a TCP and threading layer that survives long-running servo streams
  alongside concurrent dashboard service calls.

## What changed relative to upstream

### Trajectory execution
- New `TrajectoryExecutor` class (`dobot_bringup_v4/src/trajectory_executor.cpp`,
  header in `include/dobot_bringup/`): runs a fixed-rate servo loop with
  drift-compensated timing, linear interpolation between trajectory
  waypoints, robot-mode monitoring, and thread-safe access to robot
  state.
- `CRRobotRos2` now exposes a `FollowJointTrajectory` action server with
  goal / cancel / execute callbacks and configurable servo parameters
  (rate, lookahead time, gain).
- `CRCommanderRos2` gains a low-overhead `sendServoCommand()` path and a
  dedicated mutex (`dash_mutex_`) that serialises dashboard TCP access
  between ROS service handlers and the servo thread.
- Before streaming begins, the servo loop waits for the controller to
  report `ENABLE` mode (10 s timeout); ServoJ is otherwise rejected with
  error `-2` while the controller is still `RUNNING` from a previous
  command.
- Unit tests for interpolation, rad/deg conversion, and edge cases live
  under `dobot_bringup_v4/test/`.
- See `TRAJECTORY_EXECUTION.md` for the runtime design and parameter
  reference.

### TCP / threading hardening
The new servo loop exposed several latent issues in the vendor TCP code,
which were fixed:

- **Response parsing** reads from the buffer head rather than the
  advancing receive pointer; the previous behaviour silently dropped the
  error ID when a response arrived in multiple chunks.
- **Receive loop** uses a 100 ms `select` timeout plus a 2 s overall
  deadline, replacing a busy-wait that could spin indefinitely if the
  controller stopped responding.
- **Binary real-time data** is now read with a fixed-length
  `tcpRecvExact()` helper instead of scanning for a `;` terminator.
- **Locking** uses `std::lock_guard` throughout. `getRealData()` returns
  a snapshot by value rather than a shared pointer into the live buffer.
  Shadow copies of `current_joint_` and `tool_vector_` were removed;
  callers now read directly from the mutex-protected real-time data.
- **Lifetime** — the destructor guards `thread_->join()` against the
  case where `init()` was never called.
- **Robot mode** widened from `uint16_t` to `uint64_t` to match the
  underlying field width.

### MoveIt configuration (CR5)
- `cr5_moveit/config/joint_limits.yaml` populated with the actual CR5
  joint limits.
- `cr5_moveit/config/ompl_planning.yaml` extended with an `RRTstar`
  planner configuration.
- `cr5_moveit/config/stomp_planning.yaml` added.

## Tracking upstream

The fork tracks upstream `main`. README files are intentionally left
untouched apart from a one-line pointer to this document, so upstream
README edits merge cleanly. Upstream additions that do not touch the
files listed above (for example, new robot variants such as CR10AF or
new service definitions) merge without conflict.

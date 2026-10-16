# Dobot V4 protocol tests

Standalone scripts that talk to the controller directly (port 29999 for
commands, 30004 for the 8 ms realtime feed) to measure what a ros2_control
hardware interface can rely on. No ROS, Python 3.8+ standard library only;
`analyze.py` needs matplotlib.

Every script writes a timestamped directory under `--out` (default
`results/`) with a `log.txt`, `summary.json`, and for motion runs a
`commands.csv` (every ServoJ sent: time, target, reply) and `realtime.csv`
(every 30004 packet during the run). All times are this computer's
monotonic clock, so the two files line up.

## Tests

| Test | Moves the arm | Answers |
|---|---|---|
| `t1_realtime_feed.py` | no | Is the feed a steady 8 ms? Jitter, gaps, clock drift |
| `t2_connections.py` | no | Can several clients use 29999 / 30004 at once? |
| `t3_dashboard_rtt.py` | no | Command round-trip time distribution |
| `t4_pipelining.py` | no | Can commands be sent without waiting for each reply? |
| `t5_servoj_hold.py` | ServoJ, holds position | Accepted rates / t, first-command blip, ServoJ reply time |
| `t6_filter_sine.py` | yes | What aheadtime and gain do; delay and tracking error |
| `t7_timing_gaps.py` | yes | Does a late ServoJ cause a speed spike (wallclock vs index timing)? |
| `t8_start_offset.py` | yes | How far the first target may be from the actual position |
| `t9_stream_stop.py` | yes | What the arm does when the stream stops or on Stop() |
| `t10_limits.py` | yes | Which limit trips ServoJ (-2), and the alarm IDs |
| `t11_recovery.py` | enables the arm | Time from alarm to accepting ServoJ again |
| `t12_speed_scaling.py` | yes | Does SpeedFactor / the pendant slider affect ServoJ? |

Run `python3 <script> --help` for the options.

## Before running

- Stop anything else that commands the arm (the ROS driver and anything
  using it). T2 is the exception: it is meant to be run with the driver
  running too.
- For motion tests: workspace clear, e-stop in hand, collision detection on,
  joints near mid-range, the arm enabled and idle (mode 5). Start with the
  wrist (`--joint 6`, the default) and small amplitudes.
- Motion tests print their plan and ask for `yes`. They refuse profiles that
  move a joint further than `--max-excursion` (15 deg) or faster than
  `--max-speed` (60 deg/s) unless you raise those.
- Any rejected ServoJ, alarm, stale realtime data or Ctrl-C stops the stream,
  sends `Stop()` and records `GetErrorID()`.

## Suggested order

```bash
# no motion
python3 t2_connections.py --driver-running --probe-ports   # driver up
# stop the driver, then:
python3 t2_connections.py --driver-stopped
python3 t1_realtime_feed.py --duration 60
python3 t3_dashboard_rtt.py
python3 t3_dashboard_rtt.py --interval 8
python3 t4_pipelining.py

# motion, together at the arm
python3 t5_servoj_hold.py
python3 t7_timing_gaps.py
python3 t6_filter_sine.py
python3 t8_start_offset.py
python3 t9_stream_stop.py
python3 t12_speed_scaling.py
python3 t10_limits.py --vmax-list 20,40,60
python3 t11_recovery.py          # after a trip
```

## Looking at a run

```bash
python3 analyze.py results/<session>/<run> [--joint 6]
```

writes `plot_J6.png`: commanded vs `q_target` vs `q_actual`, commanded step
speed vs `qd_target` / `qd_actual`, send gaps and reply times, robot mode.

## Without the arm

`fake_controller.py` serves both ports on localhost with a crude
first-order "arm", for checking the scripts themselves (it says nothing
about the real controller):

```bash
python3 fake_controller.py [--reject-speed 50] &
python3 t7_timing_gaps.py --ip 127.0.0.1 --yes
```

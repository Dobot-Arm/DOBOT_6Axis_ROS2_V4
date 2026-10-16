# Findings so far

Session of 2026-10-09 on drover-bg (Raspberry Pi 5) against the CR5
controller at 192.168.5.1. drover-arm and drover-harvester were stopped for
all tests except the first T2 run. Raw data: `results/`.

## No-motion tests

| Test | Result |
|---|---|
| T2 connections | **Port 29999 takes one client at a time**: later connections are accepted, then closed at once; the first keeps working (driver running or not). **Port 30004 serves several clients**: three extra readers each got the full feed alongside the driver. |
| T2 other ports | 30003 refused. 30005 sends 1440-byte packets ~6/s (slow feedback feed). 30006 open, 12 bytes in 1 s, unknown. |
| T1 realtime feed | 122 Hz, **8.2 ms** period (not 8.0). Arrival on the Pi very steady: median 8.18, p99 8.56, max 9.38 ms over 60 s; no gaps. `controller_timer` steps jitter 4–13 ms and should not be used as a clock. |
| T3 command RTT | RobotMode(): median 0.8 ms, p99 2.2 ms, max 3.3 ms (back to back); p99 1.3 ms when spaced 8 ms. GetAngle(): median 1.1 ms. Arm disabled during the test. |
| T4 pipelining | Replies always in order, one per command, nothing merged. **No throughput gain**: 20 mixed queries take ~42 ms pipelined and ~40 ms one at a time (~2 ms each); the controller handles commands one after another. |

Consequences for a ros2_control driver: the hardware plugin must own the
29999 connection and provide enable / clear error / error ID itself; other
tools can read 30004 in parallel; pace the loop from 30004 packet arrival; a
wait-for-reply loop fits in an 8 ms cycle as far as RTT goes.

## Motion: the ServoJ start lurch

Safety setting on the arm: TCP speed limit 250 mm/s.

| Run | Commands (all to the current actual position) | Before it | Result |
|---|---|---|---|
| T5 `rate50_wait` (07:20) | 2 x ServoJ t=0.02, aheadtime 50, gain 500, 20 ms apart | First motion command after the arm was enabled; J1 had moved +2.28 deg since T1 (disabled, 07:05) | **Lurch + alarm 225** |
| probe (07:26) | 1 x ServoJ t=0.5 | Alarm cleared, arm re-enabled | clean |
| probe 1 (07:27) | 1 x ServoJ t=0.02 | after the probe | clean |
| probe 2 (07:27) | 2 x ServoJ t=0.02, 20 ms apart (same as T5) | after probe 1 | clean |

The lurch (T5): ~8 ms after the first ServoJ the controller's own `q_target`
started moving J1–J5 at up to ~17.8 deg/s (J1 +0.78, J2 +0.83, J3 -0.83,
J4 +0.63, J5 +0.80 deg; J6 still) and `tcp_speed_target` jumped 0 -> ~190 mm/s
in one packet. Alarm 225 at ~37 ms: pendant text "Safety TCP speed 395 mm/s
exceeding threshold 250 mm/s". The controller then decelerated its target to
a stop and the arm followed it (actual TCP speed peaked ~230 mm/s), settling
~0.8 deg away after ~140 ms. Nothing commanded motion.

Same timing (~40 ms after stream start) as the production trip.

Other observations:
- ServoJ replies carry an incrementing counter (`0,{2}`, `0,{3}`, ...),
  i.e. ServoJ appears to go through the motion queue. The first ServoJ of a
  stream takes ~10 ms to answer (queries ~1 ms); the next ~4–7 ms.
- One ServoJ keeps the arm in RUNNING(7) for about t + ~0.1 s.
- `speed_scaling` reads 99.0 (percent).

Controller log for the T5 lurch (controller clock is local time, ~10 h
ahead of the Pi's UTC journal and ~18 s behind it):

```
17:12:51  1581 front panel switch pressed / 1489 power-off signal
17:14:26  Machine ready
17:16:24  Enable robot (+ load detection, 0.20 kg)
17:16:33  Jog J1+ (twice, ~1 s)            <- the +2.28 deg on J1
17:16:44  Disable robot
17:17:01  Enable robot (+ load detection)
17:17:26  Switch on TCP mode
17:19:59  TCP connected (T5)
17:19:59  225: Safety TCP planned speed 395.117047 mm/s, approaching or
          exceeding threshold 250 mm/s
17:20:00  TCP disconnected
17:24:03  Switch off TCP mode
17:24:07  Clear error
```

So T5 was the first remote command after a power cycle, a J1+ jog, and a
disable/enable; the lurch went J1+ like the jog (but also moved J2-J5). The
alarm names the *planned* speed: the controller's planner produced it.
The clean probes came after an alarm clear and a new TCP-mode entry.

**Update 2026-10-13: cause identified (with the drover session's production
fault of 2026-10-12).** The lurch is the controller planning from an
all-zero joint reference on the first ServoJ after a power-on. In T5, at the
first moving packet (20.3 ms), J1, J4 and J5 each moved exactly 0.1562% of
their angle toward 0 (0.2287% at 28.5 ms); J2/J3 moved less because they hit
a ~17.8 deg/s cap; J6 (0.25 deg) barely moved. Every joint moved toward 0.
drover saw the same in production after a power-on (all joints ~0.08% toward
0 in 20 ms, alarm 225/227), with q_target on the arm before streaming. Both
observed cases came after a power-on, but the user reports the fault is
intermittent (not every power-on, not only after power-on is known). Cleared
by the failed stream or by the alarm/ClearError (not yet known which). The earlier hypothesis below is superseded.

Earlier working hypothesis (superseded): after the arm is moved some other way
(enable, jog, drag, pendant), the controller's servo state keeps something
stale (target or velocity) that the first ServoJ releases.

## Next

1. Keep `t1_realtime_feed.py --duration 3600` running for the whole session.
2. Find a primer that clears the zero reference. One power cycle per
   candidate: power-on -> enable -> TCP mode -> candidate ->
   `probe_servoj_start.py --servo-t 0.02 --count 2`. Candidates: zero-length
   MovJ to the current joints; ClearError() with no alarm; Stop(). (A ServoJ
   with long t is not a primer: it plans from the same zero reference.)
   Ask Dobot whether a command initialises the servo reference.
3. If it reproduces, test a pre-stream reset (Stop(), or a ServoJ hold with
   long t) and ask Dobot about it with the reproduction.
4. Then continue: T7, T6, T8, T9, T12, T10, T11.

Keep the 250 mm/s safety limit: it is what stopped the lurch.

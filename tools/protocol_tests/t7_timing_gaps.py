#!/usr/bin/env python3
"""T7: what a late ServoJ does. MOTION TEST.

Runs the same slow out-and-back move on one joint twice, holding chosen
commands back by --delays (trajectory time : extra ms):

  wallclock  each target is for the wall time of its send and the schedule
             catches up, as the fork's TrajectoryExecutor does today. A late
             send makes the next target jump ahead.
  index      command k always carries the target for k * period; a late
             send just shifts everything after it.

If qd_target spikes after the delays in 'wallclock' but not in 'index', the
timing in the current executor is a cause of the speed spikes.
"""
import os

import dobot_proto as dp


def parse_delays(text):
    out = []
    for item in text.split(','):
        if item.strip():
            t, ms = item.split(':')
            out.append((float(t), float(ms)))
    return out


def window_peak(packets, j, t0, width):
    vals = [abs(pk['qd_target'][j]) for t, pk in packets if t0 <= t <= t0 + width]
    return max(vals) if vals else None


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--amplitude', type=float, default=10.0)
    p.add_argument('--freq', type=float, default=0.25)
    p.add_argument('--cycles', type=int, default=1)
    p.add_argument('--delays', default='0.8:20,1.2:40,2.8:60',
                   help='trajectory_time:extra_ms,... (default %(default)s)')
    p.add_argument('--timings', default='wallclock,index')
    args = p.parse_args()
    s = dp.Session(args.out, 't7_timing_gaps_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    duration = args.cycles / args.freq
    period = 1.0 / args.rate
    delays = parse_delays(args.delays)
    _, spd, _ = dp.check_profile(dp.cosine_profile(q0, args.joint, args.amplitude, args.freq),
                                 duration, period, q0, args.max_excursion, args.max_speed)
    timings = [t.strip() for t in args.timings.split(',')]
    dp.confirm(['', 'Plan: J%d out %.1f deg and back over %.1f s, peak %.1f deg/s'
                % (args.joint, args.amplitude, duration, max(spd)),
                '  late sends at %s (trajectory s : extra ms)' % delays,
                '  runs: %s, rate %g Hz' % (timings, args.rate)], args.yes, s.log)

    j = args.joint - 1
    results = []
    for timing in timings:
        q = dp.idle_q(rt)
        cfg = dp.stream_config(args, timing=timing, delays=list(delays))
        summary, result, packets = dp.do_run(
            s, dash, rt, timing, dp.cosine_profile(q, args.joint, args.amplitude, args.freq),
            duration, cfg, joint=args.joint, tail=1.0)
        # Peak qd_target in the 150 ms after each late send, against the run's
        # own baseline just before it.
        rows = result['rows']
        windows = []
        for t_traj, ms in delays:
            late = next((r for r in rows if r['traj_t'] >= t_traj), None)
            if not late:
                continue
            before = window_peak(packets, j, late['t_send'] - 0.15, 0.15)
            after = window_peak(packets, j, late['t_send'], 0.15)
            windows.append({'traj_t': t_traj, 'extra_ms': ms, 'peak_qd_target_before': before,
                            'peak_qd_target_after': after,
                            'ratio': (after / before) if before and after else None})
            s.log('    late by %3.0f ms at %.1f s: |qd_target| peak %.2f before -> %.2f after (x%.2f)'
                  % (ms, t_traj, before or 0, after or 0, (after / before) if before and after else 0))
        summary['late_send_windows'] = windows
        dp.write_json(os.path.join(s.dir, timing, 'summary.json'), summary)
        results.append(dict(name=timing, **summary))
        if summary['abort']:
            s.log('stopping after an abort')
            break

    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

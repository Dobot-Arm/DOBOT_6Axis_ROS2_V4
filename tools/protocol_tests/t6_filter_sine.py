#!/usr/bin/env python3
"""T6: ServoJ filter characterization. MOTION TEST.

Moves one joint (default J6) smoothly out and back:
    q = q0 + A * (1 - cos(2*pi*f*t)) / 2       (peak speed = pi*A*f deg/s)
once for every combination of --aheadtimes x --gains.

For each run it reports the delay from the commanded position to q_target
and q_actual, the tracking error, and the peak speeds. Plot runs with
analyze.py.

Answers: what aheadtime and gain do, and which values to use.
"""
import os

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--amplitude', type=float, default=5.0, help='deg (default %(default)s)')
    p.add_argument('--freq', type=float, default=0.5, help='Hz (default %(default)s)')
    p.add_argument('--cycles', type=int, default=2)
    p.add_argument('--aheadtimes', default='20,50,100')
    p.add_argument('--gains', default='200,500,1000')
    args = p.parse_args()
    s = dp.Session(args.out, 't6_filter_sine_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    duration = args.cycles / args.freq
    period = 1.0 / args.rate
    profile = dp.cosine_profile(q0, args.joint, args.amplitude, args.freq)
    exc, spd, _ = dp.check_profile(profile, duration, period, q0, args.max_excursion, args.max_speed)
    combos = [(a, g) for a in dp.parse_list(args.aheadtimes) for g in dp.parse_list(args.gains)]
    dp.confirm(['', 'Plan: J%d from %.2f to %.2f deg and back, %d cycle(s) of %.1f s, peak %.1f deg/s'
                % (args.joint, q0[args.joint - 1], q0[args.joint - 1] + args.amplitude, args.cycles,
                   1 / args.freq, max(spd)),
                '  %d runs: (aheadtime, gain) = %s' % (len(combos), combos),
                '  rate %g Hz, t=%.4f s' % (args.rate, args.servo_t or period)],
               args.yes, s.log)

    results = []
    for a, g in combos:
        q = dp.idle_q(rt)
        name = 'ahead%g_gain%g' % (a, g)
        cfg = dp.stream_config(args, aheadtime=a, gain=g)
        summary, _, _ = dp.do_run(s, dash, rt, name,
                                  dp.cosine_profile(q, args.joint, args.amplitude, args.freq),
                                  duration, cfg, joint=args.joint, tail=1.0)
        results.append(dict(name=name, aheadtime=a, gain=g, **summary))
        if summary['abort']:
            s.log('stopping after an abort')
            break

    s.log('\n%-22s %10s %10s %10s %12s' % ('run', 'lag q_tgt', 'lag q_act', 'rms q_act', 'peak qd_tgt'))
    for r in results:
        s.log('%-22s %8s ms %8s ms %8s deg %8.2f deg/s'
              % (r['name'], r.get('lag_q_target_ms'), r.get('lag_q_actual_ms'),
                 r.get('rms_q_actual_deg'), r.get('peak_qd_target_dps', float('nan'))))
    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

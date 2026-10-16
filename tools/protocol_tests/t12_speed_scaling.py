#!/usr/bin/env python3
"""T12: does the global speed setting affect ServoJ? MOTION TEST.

Runs the T6 move once per value in --speed-factors, setting it with
SpeedFactor(N) before each run and restoring the original afterwards. With
--manual it doesn't set anything: it pauses before each run so you can move
the pendant's speed slider, and records what the realtime feed reports.

Compares the commanded speed with qd_target and the delay, and logs the
speed_scaling field the controller reports.

Answers: whether ServoJ obeys the speed setting, and whether speed_scaling
can drive a scaled trajectory controller.
"""
import os
import time

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--amplitude', type=float, default=5.0)
    p.add_argument('--freq', type=float, default=0.5)
    p.add_argument('--cycles', type=int, default=2)
    p.add_argument('--speed-factors', default='100,50', help='percent (default %(default)s)')
    p.add_argument('--manual', type=int, default=0,
                   help='instead of SpeedFactor: N runs, pausing for the pendant slider before each')
    args = p.parse_args()
    s = dp.Session(args.out, 't12_speed_scaling_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    duration = args.cycles / args.freq
    _, spd, _ = dp.check_profile(dp.cosine_profile(q0, args.joint, args.amplitude, args.freq),
                                 duration, 1.0 / args.rate, q0, args.max_excursion, args.max_speed)
    original = rt.current()['speed_scaling']
    if args.manual:
        plan = ['  %d runs; set the pendant speed slider before each' % args.manual]
        runs = list(range(args.manual))
    else:
        runs = dp.parse_list(args.speed_factors, int)
        plan = ['  SpeedFactor values %s, then restore (speed_scaling now reads %s)' % (runs, original)]
    dp.confirm(['', 'Plan: J%d out %.1f deg and back, %d cycle(s), peak %.1f deg/s'
                % (args.joint, args.amplitude, args.cycles, max(spd))] + plan, args.yes, s.log)

    results = []
    try:
        for r in runs:
            if args.manual:
                input('  set the speed slider for run %d, then press Enter: ' % (r + 1))
                name = 'manual%d' % (r + 1)
            else:
                reply = dash.query('SpeedFactor(%d)' % r)
                s.log('  SpeedFactor(%d) -> %s' % (r, reply.raw))
                name = 'speed%d' % r
                time.sleep(0.2)  # let the realtime feed catch up
            scaling = rt.current()['speed_scaling']
            s.log('  speed_scaling reads %s' % scaling)
            q = dp.idle_q(rt)
            summary, _, _ = dp.do_run(s, dash, rt, name,
                                      dp.cosine_profile(q, args.joint, args.amplitude, args.freq),
                                      duration, dp.stream_config(args), joint=args.joint, tail=1.0)
            summary['speed_scaling_before'] = scaling
            dp.write_json(os.path.join(s.dir, name, 'summary.json'), summary)
            results.append(dict(name=name, **summary))
            if summary['abort']:
                s.log('stopping after an abort')
                break
    finally:
        if not args.manual:
            # speed_scaling may read as a fraction or as a percentage.
            restore = int(round(original * 100 if original <= 1.0 else original))
            reply = dash.query('SpeedFactor(%d)' % restore)
            s.log('restored SpeedFactor(%d) -> %s' % (restore, reply.raw))

    s.log('\n%-10s %14s %16s %12s' % ('run', 'speed_scaling', 'peak qd_target', 'lag q_tgt'))
    for r in results:
        s.log('%-10s %14s %12.2f deg/s %9s ms' % (r['name'], r['speed_scaling_before'],
                                                  r.get('peak_qd_target_dps', float('nan')),
                                                  r.get('lag_q_target_ms')))
    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

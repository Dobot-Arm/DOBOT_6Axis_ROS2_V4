#!/usr/bin/env python3
"""T8: starting a stream away from the actual position. MOTION TEST.

For each offset in --offsets, streams ServoJ to (actual position + offset)
on one joint for --hold seconds: the very first command asks for a step.
Signs alternate so the joint goes back and forth around where it started.
Stops at the first rejection or alarm.

Answers: how far the first ServoJ target may be from the actual position
before the controller spikes or refuses, i.e. how carefully a driver has to
start a stream.
"""
import os

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--offsets', default='0.05,0.1,0.25,0.5,1.0', help='deg (default %(default)s)')
    p.add_argument('--hold', type=float, default=1.0, help='s per run (default %(default)s)')
    args = p.parse_args()
    s = dp.Session(args.out, 't8_start_offset_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    dp.preflight(dash, rt, s.log)

    offsets = dp.parse_list(args.offsets)
    if max(offsets) > args.max_excursion:
        raise SystemExit('largest offset exceeds --max-excursion')
    dp.confirm(['', 'Plan: J%d, first ServoJ target offset from actual by %s deg (alternating sign),'
                % (args.joint, offsets),
                '  hold %.1f s each, rate %g Hz; stops at the first rejection' % (args.hold, args.rate)],
               args.yes, s.log)

    j = args.joint - 1
    results = []
    for i, off in enumerate(offsets):
        sign = 1 if i % 2 == 0 else -1
        q = dp.idle_q(rt)
        target = list(q)
        target[j] += sign * off
        name = 'offset%+g' % (sign * off)
        cfg = dp.stream_config(args)
        summary, result, packets = dp.do_run(s, dash, rt, name, dp.hold_profile(target),
                                             args.hold, cfg, joint=args.joint)
        t_first = result['rows'][0]['t_send'] if result['rows'] else None
        early = [abs(pk['qd_target'][j]) for t, pk in packets if t_first and t_first <= t <= t_first + 0.2]
        summary['offset_deg'] = sign * off
        summary['first_200ms_peak_qd_target_dps'] = max(early) if early else None
        s.log('    offset %+.2f deg: peak |qd_target| in first 200 ms %s deg/s'
              % (sign * off, summary['first_200ms_peak_qd_target_dps']))
        dp.write_json(os.path.join(s.dir, name, 'summary.json'), summary)
        results.append(dict(name=name, **summary))
        if summary['abort'] or summary['rejected']:
            s.log('stopping at the first rejection')
            break

    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""T10: which limit trips ServoJ, and where. MOTION TEST.

Moves one joint out by --distance and back with a trapezoidal velocity
profile, sweeping either
  --vmax-list at a fixed --amax       (speed sweep), or
  --amax-list at a fixed --vmax       (acceleration sweep).
Stops at the first rejection or alarm, then records GetErrorID together with
the joint's commanded and actual speed and acceleration at that moment.

The arm is left in its error state after a trip; clear it on the pendant or
with t11_recovery.py.

Answers: what the -2 rejections actually are, and the real limits to put in
joint_limits.yaml.
"""
import os

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--distance', type=float, default=30.0, help='deg (default %(default)s)')
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument('--vmax-list', help='deg/s, comma separated; sweeps speed at --amax')
    g.add_argument('--amax-list', help='deg/s^2, comma separated; sweeps acceleration at --vmax')
    p.add_argument('--vmax', type=float, default=30.0, help='deg/s for the acceleration sweep')
    p.add_argument('--amax', type=float, default=100.0, help='deg/s^2 for the speed sweep')
    p.add_argument('--pause', type=float, default=0.3, help='s at the far end')
    args = p.parse_args()
    s = dp.Session(args.out, 't10_limits_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    if args.vmax_list:
        sweep = [(v, args.amax) for v in dp.parse_list(args.vmax_list)]
        kind = 'speed sweep at amax=%g deg/s^2' % args.amax
    else:
        sweep = [(args.vmax, a) for a in dp.parse_list(args.amax_list)]
        kind = 'acceleration sweep at vmax=%g deg/s' % args.vmax
    period = 1.0 / args.rate
    for v, a in sweep:
        prof, dur = dp.out_and_back_profile(q0, args.joint, args.distance, v, a, args.pause)
        dp.check_profile(prof, dur, period, q0, args.max_excursion, args.max_speed)
    dp.confirm(['', 'Plan: J%d out %.1f deg and back, %s' % (args.joint, args.distance, kind),
                '  (vmax, amax) = %s' % sweep,
                '  stops at the first rejection and leaves the arm in its error state'],
               args.yes, s.log)

    j = args.joint - 1
    results = []
    for v, a in sweep:
        q = dp.idle_q(rt)
        prof, dur = dp.out_and_back_profile(q, args.joint, args.distance, v, a, args.pause)
        name = 'v%g_a%g' % (v, a)
        summary, result, packets = dp.do_run(s, dash, rt, name, prof, dur,
                                             dp.stream_config(args), joint=args.joint, tail=1.0)
        summary.update({'vmax': v, 'amax': a})
        if summary['abort']:
            # State of the joint around the trip: the last commands and packets.
            t_trip = next((r['t_reply'] for r in result['rows'] if r['err'] not in (0, None)), None)
            if t_trip is None:
                t_trip = result['t_end']
            near = [pk for t, pk in packets if t_trip - 0.1 <= t <= t_trip]
            if near:
                summary['at_trip'] = {
                    'peak_qd_target_dps': max(abs(pk['qd_target'][j]) for pk in near),
                    'peak_qdd_target_dps2': max(abs(pk['qdd_target'][j]) for pk in near),
                    'peak_qd_actual_dps': max(abs(pk['qd_actual'][j]) for pk in near),
                    'max_following_error_deg': max(abs(pk['q_target'][j] - pk['q_actual'][j]) for pk in near),
                }
                s.log('    at trip (last 100 ms): %s' % summary['at_trip'])
        dp.write_json(os.path.join(s.dir, name, 'summary.json'), summary)
        results.append(dict(name=name, **summary))
        if summary['abort']:
            s.log('stopping at the first trip')
            break

    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""T5: ServoJ holding the current position. MOTION TEST (no intended motion).

Streams ServoJ to the arm's current actual position for --duration seconds
at each rate in --rates (t = 1/rate), waiting for each reply and, unless
--no-pipelined, again without waiting. The arm should not move.

Answers:
  - which rates / t values the controller accepts
  - whether the FIRST ServoJ after idle causes a blip (qd_target in the
    first 100 ms)
  - ServoJ reply time while streaming, waiting vs pipelined
"""
import os

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--rates', default='50,125,250', help='Hz, comma separated (default %(default)s)')
    p.add_argument('--duration', type=float, default=3.0)
    p.add_argument('--no-pipelined', action='store_true', help='only the wait-for-reply mode')
    args = p.parse_args()
    s = dp.Session(args.out, 't5_servoj_hold', args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    rates = dp.parse_list(args.rates)
    modes = [False] if args.no_pipelined else [False, True]
    dp.confirm(['', 'Plan: hold the current position with ServoJ, %.1f s per run' % args.duration,
                '  rates %s Hz, %s' % (rates, 'wait + pipelined' if len(modes) == 2 else 'wait for reply'),
                '  the arm should not move'], args.yes, s.log)

    results = []
    for rate in rates:
        for pipelined in modes:
            name = 'rate%g_%s' % (rate, 'pipe' if pipelined else 'wait')
            try:
                q = dp.idle_q(rt)
            except RuntimeError as e:
                s.log('stopping: %s' % e)
                break
            cfg = dp.stream_config(args, rate=rate, servo_t=1.0 / rate, pipelined=pipelined)
            summary, result, packets = dp.do_run(s, dash, rt, name, dp.hold_profile(q),
                                                 args.duration, cfg)
            # Blip check: largest joint speed / position change in the first 100 ms.
            t_first = result['rows'][0]['t_send'] if result['rows'] else None
            early = [pk for t, pk in packets if t_first and t_first <= t <= t_first + 0.1]
            moving = [pk for t, pk in packets if t_first and t >= t_first]
            summary['first_100ms_peak_qd_target_dps'] = max(
                (abs(v) for pk in early for v in pk['qd_target']), default=None)
            summary['peak_qd_target_dps'] = max(
                (abs(v) for pk in moving for v in pk['qd_target']), default=None)
            summary['max_q_actual_drift_deg'] = max(
                (abs(pk['q_actual'][j] - q[j]) for pk in moving for j in range(6)), default=None)
            s.log('    first 100 ms peak |qd_target| %s deg/s, whole run %s deg/s, '
                  'max drift of q_actual %s deg'
                  % (summary['first_100ms_peak_qd_target_dps'], summary['peak_qd_target_dps'],
                     summary['max_q_actual_drift_deg']))
            dp.write_json(os.path.join(s.dir, name, 'summary.json'), summary)
            results.append(dict(name=name, **summary))
            if summary['abort']:
                s.log('stopping after an abort')
                break
        else:
            continue
        break

    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

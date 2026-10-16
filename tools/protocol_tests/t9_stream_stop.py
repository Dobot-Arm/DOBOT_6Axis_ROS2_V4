#!/usr/bin/env python3
"""T9: what the arm does when the stream stops mid-move. MOTION TEST.

Starts a slow out-and-back move on one joint and, at --stop-at seconds
(default: the moment of peak speed), either
  quiet  stops sending (as if the computer crashed or the link stalled), or
  stop   sends Stop()
then keeps recording for --observe seconds.

Reports how long the joint took to come to rest after the last command, how
far it travelled, and the robot mode and alarms afterwards.

Answers: safety behaviour a driver must expect and handle.
"""
import os
import time

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--joint', type=int, default=6)
    p.add_argument('--amplitude', type=float, default=10.0)
    p.add_argument('--freq', type=float, default=0.25)
    p.add_argument('--stop-at', type=float, default=None,
                   help='trajectory time to stop, s (default: peak speed, 1/(4f))')
    p.add_argument('--modes', default='quiet,stop')
    p.add_argument('--observe', type=float, default=3.0)
    p.add_argument('--rest-speed', type=float, default=0.1, help='deg/s counted as at rest')
    args = p.parse_args()
    s = dp.Session(args.out, 't9_stream_stop_J%d' % args.joint, args)
    dash, rt = dp.connect(args, s.log)
    q0 = dp.preflight(dash, rt, s.log)

    stop_at = args.stop_at if args.stop_at is not None else 1.0 / (4 * args.freq)
    duration = 1.0 / args.freq
    period = 1.0 / args.rate
    _, spd, _ = dp.check_profile(dp.cosine_profile(q0, args.joint, args.amplitude, args.freq),
                                 duration, period, q0, args.max_excursion, args.max_speed)
    modes = [m.strip() for m in args.modes.split(',')]
    dp.confirm(['', 'Plan: J%d out %.1f deg and back, peak %.1f deg/s; stop sending at %.2f s'
                % (args.joint, args.amplitude, max(spd), stop_at),
                '  modes: %s (quiet = just stop sending, stop = send Stop())' % modes,
                '  then watch for %.1f s' % args.observe], args.yes, s.log)

    j = args.joint - 1
    results = []
    for mode in modes:
        q = dp.idle_q(rt)
        cfg = dp.stream_config(args, stop_at=stop_at, stop_cmd=(mode == 'stop'))
        i0 = rt.index()
        s.log('  run %s' % mode)
        result = dp.run_stream(dash, rt, dp.cosine_profile(q, args.joint, args.amplitude, args.freq),
                               duration, cfg, log=s.log)
        t_last = result['rows'][-1]['t_send'] if result['rows'] else dp.now()
        last_target = result['rows'][-1]['target'][j] if result['rows'] else q[j]
        time.sleep(args.observe)
        pk_now = rt.current(max_age=0.5)
        mode_after = pk_now['robot_mode']
        errors = dash.query('GetErrorID()').raw
        packets = rt.slice(i0)

        after = [(t, pk) for t, pk in packets if t >= t_last]
        moving = [t for t, pk in after if abs(pk['qd_actual'][j]) > args.rest_speed]
        rest_t = (moving[-1] - t_last) if moving else 0.0
        q_at_last = next((pk['q_actual'][j] for t, pk in after), None)
        q_final = after[-1][1]['q_actual'][j] if after else None
        summary = dp.stream_summary(result, packets, joint=args.joint)
        summary.update({
            'mode': mode, 'stop_at': stop_at,
            'time_to_rest_after_last_command_s': rest_t,
            'travel_after_last_command_deg': (q_final - q_at_last) if q_final is not None and q_at_last is not None else None,
            'final_minus_last_target_deg': (q_final - last_target) if q_final is not None else None,
            'robot_mode_after': dp.mode_name(mode_after),
            'error_ids_after': errors,
            'modes_seen_after': sorted({dp.mode_name(pk['robot_mode']) for _, pk in after}),
        })
        s.log('    at rest %.3f s after the last command; travelled %s deg after it; '
              'final - last target %s deg'
              % (rest_t, summary['travel_after_last_command_deg'], summary['final_minus_last_target_deg']))
        s.log('    mode afterwards %s (seen: %s); GetErrorID %s'
              % (summary['robot_mode_after'], summary['modes_seen_after'], errors))
        s.save_run(mode, result, packets, summary)
        results.append(summary)

        # Leave the arm idle for the next run.
        if mode_after not in (5,) + dp.ERROR_MODES:
            dash.query('Stop()')
        final = dp.wait_idle(dash, rt, log=s.log)
        if final != 5:
            s.log('arm is %s; stopping. Clear it before running more tests.' % dp.mode_name(final))
            break

    dp.write_json(os.path.join(s.dir, 'summary.json'), results)
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

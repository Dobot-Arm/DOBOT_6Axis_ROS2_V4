#!/usr/bin/env python3
"""Probe: what the controller does on the first ServoJ(s) after idle. MOTION TEST.

Sends --count ServoJ commands (default 1), all to the arm's CURRENT actual
position, --interval s apart, with the given t / aheadtime / gain. Records
--before s of the feed before the first command and --after s after the last,
then prints, per packet: robot mode, q_target and q_actual relative to the
commanded position, and the controller's target TCP speed.

Nothing in the command asks for motion, so any change in q_target is the
controller's own doing.
"""
import math
import os
import time

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--count', type=int, default=1)
    p.add_argument('--interval', type=float, default=0.02)
    p.add_argument('--before', type=float, default=0.3)
    p.add_argument('--after', type=float, default=2.0)
    p.set_defaults(servo_t=0.5)
    args = p.parse_args()
    s = dp.Session(args.out, 'probe_servoj_start', args)
    dash, rt = dp.connect(args, s.log)
    q = dp.preflight(dash, rt, s.log)
    dp.confirm(['', 'Plan: %d ServoJ to the current position, t=%g aheadtime=%g gain=%g, %g s apart'
                % (args.count, args.servo_t, args.aheadtime, args.gain, args.interval),
                '  the arm should not move'], args.yes, s.log)

    i0 = rt.index()
    time.sleep(args.before)
    cmd = dp.servoj_cmd(q, args.servo_t, args.aheadtime, args.gain)
    replies = []
    for _ in range(args.count):
        r = dash.query(cmd)
        replies.append({'t_send': r.t_send, 't_recv': r.t_recv, 'rtt_ms': r.rtt_ms, 'reply': r.raw})
        s.log('sent %s -> %s (%.1f ms)' % (cmd, r.raw, r.rtt_ms))
        if r.err != 0:
            break
        time.sleep(args.interval)
    time.sleep(args.after)
    pkt = rt.current(max_age=0.5)
    errors = dash.query('GetErrorID()').raw
    s.log('mode after: %s   GetErrorID: %s' % (dp.mode_name(pkt['robot_mode']), errors))
    if pkt['robot_mode'] == 7:
        s.log('still RUNNING: Stop() -> %s' % dash.query('Stop()').raw)
    packets = rt.slice(i0)
    rt.stop()
    dash.close()

    t0 = replies[0]['t_send']
    s.log('\n   t ms  mode  q_target - cmd (J1..J6)                          '
          'q_actual - cmd                                    |tcp_v_target| |tcp_v_actual| mm/s')
    last = None
    peak = {'q_target_dev_deg': 0.0, 'tcp_speed_target': 0.0, 'q_actual_dev_deg': 0.0}
    for t, pk in packets:
        dt = (t - t0) * 1e3
        qt = [a - b for a, b in zip(pk['q_target'], q)]
        qa = [a - b for a, b in zip(pk['q_actual'], q)]
        vt = math.sqrt(sum(v * v for v in pk['tcp_speed_target'][:3]))
        va = math.sqrt(sum(v * v for v in pk['tcp_speed_actual'][:3]))
        peak['q_target_dev_deg'] = max(peak['q_target_dev_deg'], max(abs(v) for v in qt))
        peak['q_actual_dev_deg'] = max(peak['q_actual_dev_deg'], max(abs(v) for v in qa))
        peak['tcp_speed_target'] = max(peak['tcp_speed_target'], vt)
        line = '%7.1f  %4s  %s   %s   %8.1f %8.1f' % (
            dt, pk['robot_mode'], ' '.join('%+.4f' % v for v in qt),
            ' '.join('%+.4f' % v for v in qa), vt, va)
        # Print every packet near the start, then only when something changes.
        key = (pk['robot_mode'], tuple(round(v, 3) for v in qt), round(vt))
        if -50 <= dt <= 300 or key != last:
            s.log(line)
        last = key
    s.log('\npeak |q_target - cmd| %.4f deg, peak |q_actual - cmd| %.4f deg, peak target TCP speed %.1f mm/s'
          % (peak['q_target_dev_deg'], peak['q_actual_dev_deg'], peak['tcp_speed_target']))

    dp.write_realtime_csv(os.path.join(s.dir, 'realtime.csv'), packets)
    dp.write_json(os.path.join(s.dir, 'summary.json'),
                  {'commanded': q, 'cmd': cmd, 'replies': replies, 'errors_after': errors,
                   'mode_after': dp.mode_name(pkt['robot_mode']), 'peak': peak})


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""T11: recovering from an alarm. MOTION TEST (enabling releases the brakes).

Times each step of getting the arm back to accepting ServoJ:
    GetErrorID -> ClearError() -> EnableRobot() (if disabled) -> ENABLE(5)
    -> ServoJ holding the current position for --hold s
Works from ERROR, DISABLED or already ENABLE.

Answers: how long recovery takes, and whether the first ServoJ after it
behaves (compare with T5 and T8).
"""
import os
import time

import dobot_proto as dp


def wait_mode(rt, wanted, timeout):
    t0 = dp.now()
    while dp.now() - t0 < timeout:
        m = rt.current(max_age=0.5)['robot_mode']
        if m in wanted:
            return m, dp.now() - t0
        time.sleep(0.01)
    return rt.current(max_age=0.5)['robot_mode'], None


def main():
    p = dp.base_parser(__doc__.splitlines()[0], motion=True)
    p.add_argument('--hold', type=float, default=1.0)
    p.add_argument('--timeout', type=float, default=20.0)
    args = p.parse_args()
    s = dp.Session(args.out, 't11_recovery', args)
    dash, rt = dp.connect(args, s.log)

    mode = rt.current()['robot_mode']
    s.log('mode now: %s' % dp.mode_name(mode))
    dp.confirm(['', 'Plan: clear alarms, enable the arm if needed (brakes release), then hold '
                'position with ServoJ for %.1f s' % args.hold], args.yes, s.log)

    steps = []
    t_start = dp.now()
    i0 = rt.index()

    def step(name, cmd=None, wait_for=None):
        entry = {'step': name}
        if cmd:
            r = dash.query(cmd)
            entry.update({'cmd': cmd, 'reply': r.raw, 'rtt_ms': round(r.rtt_ms, 2)})
        if wait_for:
            m, dt = wait_mode(rt, wait_for, args.timeout)
            entry.update({'mode': dp.mode_name(m), 'reached_after_s': dt})
        entry['t'] = round(dp.now() - t_start, 3)
        steps.append(entry)
        s.log('  %-10s t=%6.3f s  %s' % (name, entry['t'],
                                          {k: v for k, v in entry.items() if k not in ('step', 't')}))
        return entry

    step('errors', 'GetErrorID()')
    if mode in dp.ERROR_MODES or rt.current()['error_status']:
        step('clear', 'ClearError()', wait_for={4, 5})
    if rt.current()['robot_mode'] == 4:
        step('enable', 'EnableRobot()', wait_for={5})
    m, _ = wait_mode(rt, {5}, args.timeout)
    if m != 5:
        s.log('arm did not reach ENABLE(5): %s' % dp.mode_name(m))
        dp.write_json(os.path.join(s.dir, 'summary.json'), {'steps': steps})
        return
    entry = {'step': 'ready', 't': round(dp.now() - t_start, 3)}
    steps.append(entry)
    s.log('  ready      t=%6.3f s' % entry['t'])

    q = dp.idle_q(rt)
    summary, result, packets = dp.do_run(s, dash, rt, 'first_servoj', dp.hold_profile(q),
                                         args.hold, dp.stream_config(args))
    t_first = result['rows'][0]['t_send'] if result['rows'] else None
    early = [abs(v) for t, pk in packets if t_first and t_first <= t <= t_first + 0.1
             for v in pk['qd_target']]
    summary['first_100ms_peak_qd_target_dps'] = max(early) if early else None
    s.log('    first ServoJ: %s, first 100 ms peak |qd_target| %s deg/s'
          % (result['rows'][0]['reply'] if result['rows'] else None,
             summary['first_100ms_peak_qd_target_dps']))
    dp.write_realtime_csv(os.path.join(s.dir, 'realtime_all.csv'), rt.slice(i0))
    dp.write_json(os.path.join(s.dir, 'summary.json'), {'steps': steps, 'first_servoj': summary})
    rt.stop()
    dash.close()


if __name__ == '__main__':
    main()

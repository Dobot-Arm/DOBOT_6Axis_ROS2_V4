#!/usr/bin/env python3
"""T3: command port round-trip time. No motion.

Sends a read-only query (default RobotMode()) --count times, waiting for
each reply, and reports the round-trip time distribution. --interval adds a
pause between queries (e.g. 8 ms to mimic a control loop instead of
back-to-back).

Answers: can a loop that waits for every reply run at 125 Hz (8 ms)?
"""
import csv
import os
import time

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0])
    p.add_argument('--count', type=int, default=1000)
    p.add_argument('--cmd', default='RobotMode()', help='read-only query (default %(default)s)')
    p.add_argument('--interval', type=float, default=0.0, help='pause between queries, ms')
    args = p.parse_args()
    s = dp.Session(args.out, 't3_dashboard_rtt', args)

    dash = dp.Dashboard(args.ip)
    expected = args.cmd.split('(', 1)[0]
    rows, bad = [], 0
    s.log('%d x %s, interval %.1f ms...' % (args.count, args.cmd, args.interval))
    for i in range(args.count):
        r = dash.query(args.cmd)
        ok = r.err == 0 and r.echo == expected
        bad += not ok
        rows.append((i, r.t_send, r.t_recv, r.rtt_ms, r.err, ok, r.raw))
        if args.interval:
            time.sleep(args.interval / 1e3)
    dash.close()

    with open(os.path.join(s.dir, 'rtt.csv'), 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['i', 't_send', 't_recv', 'rtt_ms', 'err', 'ok', 'reply'])
        w.writerows(rows)

    rtt = [r[3] for r in rows]
    st = dp.stats(rtt)
    over = {ms: sum(1 for v in rtt if v > ms) for ms in (4, 8, 16, 50)}
    summary = {'cmd': args.cmd, 'interval_ms': args.interval, 'rtt_ms': st,
               'over_ms': over, 'bad_replies': bad, 'example_reply': rows[0][6]}
    dp.write_json(os.path.join(s.dir, 'summary.json'), summary)
    s.log('example reply: %s' % rows[0][6])
    s.log('RTT: %s' % dp.fmt_stats(st))
    s.log('over 4 ms: %d, over 8 ms: %d, over 16 ms: %d, over 50 ms: %d'
          % (over[4], over[8], over[16], over[50]))
    s.log('unexpected replies: %d' % bad)


if __name__ == '__main__':
    main()

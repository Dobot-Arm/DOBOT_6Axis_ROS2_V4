#!/usr/bin/env python3
"""T4: pipelining on the command port. No motion.

Sends a burst of --burst read-only queries back to back without waiting,
then reads the replies. Repeats --rounds times. Each reply echoes the name of
its command, so the test can check that every query got exactly one reply,
in order, and that none were merged or garbled.

Compares the burst time with sending the same queries one at a time.

Answers: can a control loop send the next command without waiting for the
previous reply? (Checked with harmless queries before trying it with ServoJ
in T5.)
"""
import os

import dobot_proto as dp

QUERIES = ['RobotMode()', 'GetErrorID()', 'GetAngle()', 'GetPose()']


def main():
    p = dp.base_parser(__doc__.splitlines()[0])
    p.add_argument('--burst', type=int, default=20)
    p.add_argument('--rounds', type=int, default=20)
    args = p.parse_args()
    s = dp.Session(args.out, 't4_pipelining', args)

    dash = dp.Dashboard(args.ip)
    # Each query must be known to the controller before we rely on it.
    for q in QUERIES:
        r = dash.query(q)
        s.log('  %-14s -> %s' % (q, r.raw))
    usable = [q for q in QUERIES if dash.query(q).err == 0]
    if len(usable) < 2:
        s.log('fewer than two usable queries, cannot check ordering')
        return

    rounds = []
    for n in range(args.rounds):
        cmds = [usable[i % len(usable)] for i in range(args.burst)]
        t0 = dp.now()
        sends = [dash.send(c) for c in cmds]
        replies = [dash.recv_reply() for _ in cmds]
        t1 = dp.now()
        order_ok = all(dp.Reply(raw, 0, 0).echo == c.split('(')[0] for c, (raw, _) in zip(cmds, replies))
        errors = sum(1 for raw, _ in replies if dp.parse_reply(raw)[0] != 0)
        first_reply_ms = (replies[0][1] - sends[0]) * 1e3
        rounds.append({'burst_ms': (t1 - t0) * 1e3, 'order_ok': order_ok, 'errors': errors,
                       'first_reply_ms': first_reply_ms,
                       'mismatches': [(c, raw) for c, (raw, _) in zip(cmds, replies)
                                      if dp.Reply(raw, 0, 0).echo != c.split('(')[0]][:5]})

    # Same queries, one at a time, for comparison.
    seq = []
    for n in range(args.rounds):
        t0 = dp.now()
        for i in range(args.burst):
            dash.query(usable[i % len(usable)])
        seq.append((dp.now() - t0) * 1e3)

    # Nothing extra may be left over on the socket.
    dash.sock.settimeout(0.5)
    try:
        leftover = dash.recv_reply()[0]
    except Exception:
        leftover = None
    dash.close()

    burst = dp.stats([r['burst_ms'] for r in rounds])
    sequential = dp.stats(seq)
    summary = {
        'queries': usable, 'burst': args.burst, 'rounds': args.rounds,
        'all_in_order': all(r['order_ok'] for r in rounds),
        'rounds_out_of_order': sum(1 for r in rounds if not r['order_ok']),
        'error_replies': sum(r['errors'] for r in rounds),
        'pipelined_burst_ms': burst,
        'sequential_burst_ms': sequential,
        'first_reply_in_burst_ms': dp.stats([r['first_reply_ms'] for r in rounds]),
        'leftover_after_test': leftover,
        'mismatch_examples': [m for r in rounds for m in r['mismatches']][:10],
    }
    dp.write_json(os.path.join(s.dir, 'summary.json'), summary)
    s.log('replies in order, one per query: %s (%d of %d rounds out of order)'
          % (summary['all_in_order'], summary['rounds_out_of_order'], args.rounds))
    s.log('error replies: %d   leftover bytes after test: %r' % (summary['error_replies'], leftover))
    s.log('%d queries pipelined:      %s' % (args.burst, dp.fmt_stats(burst)))
    s.log('%d queries one at a time:  %s' % (args.burst, dp.fmt_stats(sequential)))
    if summary['mismatch_examples']:
        s.log('mismatches: %s' % summary['mismatch_examples'])


if __name__ == '__main__':
    main()

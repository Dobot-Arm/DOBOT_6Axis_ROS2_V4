#!/usr/bin/env python3
"""T2: can several clients connect at once? No motion.

Opens up to --max connections to the command port (29999), one at a time,
and after each one checks that EVERY open connection still answers
RobotMode(). Then does the same for the realtime port (30004), checking
that every open connection keeps receiving packets.

Only read-only queries are sent. Run it twice if you can: once with the ROS
driver running (it already holds one connection to each port) and once with
it stopped, and pass --driver-running / --driver-stopped so the result says
which it was.

With --probe-ports it also tries to connect to the other ports Dobot
controllers have used (30003, 30005, 30006) and reports how much data each
sends in one second, without sending anything.
"""
import os
import socket
import time

import dobot_proto as dp


def check_dash(conns, log):
    out = []
    for i, c in enumerate(conns):
        try:
            r = c.query('RobotMode()')
            out.append({'conn': i + 1, 'ok': r.err == 0, 'reply': r.raw, 'rtt_ms': round(r.rtt_ms, 2)})
            log('    connection %d: %s  (%.1f ms)' % (i + 1, r.raw, r.rtt_ms))
        except Exception as e:
            out.append({'conn': i + 1, 'ok': False, 'error': '%s: %s' % (type(e).__name__, e)})
            log('    connection %d: FAILED %s: %s' % (i + 1, type(e).__name__, e))
    return out


def test_dashboard(args, log):
    log('\n== command port %d ==' % dp.DASH_PORT)
    conns, steps = [], []
    for n in range(1, args.max + 1):
        log('  opening connection %d...' % n)
        step = {'opened': n}
        try:
            conns.append(dp.Dashboard(args.ip))
            step['connect'] = 'ok'
        except Exception as e:
            step['connect'] = '%s: %s' % (type(e).__name__, e)
            log('    connect failed: %s' % step['connect'])
            steps.append(step)
            break
        time.sleep(0.3)
        step['checks'] = check_dash(conns, log)
        steps.append(step)
    # Re-check after a pause: some servers drop the older client a bit later.
    time.sleep(2.0)
    log('  re-check after 2 s:')
    steps.append({'recheck': check_dash(conns, log)})
    for c in conns:
        c.close()
    return steps


def test_realtime(args, log):
    log('\n== realtime port %d ==' % dp.RT_PORT)
    readers, steps = [], []
    for n in range(1, args.max + 1):
        log('  opening connection %d...' % n)
        step = {'opened': n}
        try:
            r = dp.RealtimeReader(args.ip, keep=False)
            r.start()
            readers.append(r)
            step['connect'] = 'ok'
        except Exception as e:
            step['connect'] = '%s: %s' % (type(e).__name__, e)
            log('    connect failed: %s' % step['connect'])
            steps.append(step)
            break
        before = [rd.count for rd in readers]
        time.sleep(2.0)
        checks = []
        for i, rd in enumerate(readers):
            got = rd.count - before[i]
            checks.append({'conn': i + 1, 'packets_in_2s': got,
                           'error': str(rd.error) if rd.error else None})
            log('    connection %d: %d packets in 2 s%s'
                % (i + 1, got, ('  ERROR %s' % rd.error) if rd.error else ''))
        step['checks'] = checks
        steps.append(step)
    for rd in readers:
        rd.stop()
    return steps


def probe_ports(args, log):
    log('\n== other ports (connect and listen 1 s, nothing sent) ==')
    out = []
    for port in (30003, 30005, 30006):
        entry = {'port': port}
        try:
            s = socket.create_connection((args.ip, port), timeout=2.0)
            s.settimeout(0.2)
            total, first = 0, None
            t_end = dp.now() + 1.0
            while dp.now() < t_end:
                try:
                    chunk = s.recv(65536)
                    if not chunk:
                        entry['closed_by_peer'] = True
                        break
                    if first is None:
                        first = chunk[:16].hex()
                    total += len(chunk)
                except socket.timeout:
                    pass
            s.close()
            entry.update({'connect': 'ok', 'bytes_in_1s': total, 'first_bytes_hex': first})
            log('  %d: connected, %d bytes in 1 s%s' % (port, total,
                                                        ' (closed by controller)' if entry.get('closed_by_peer') else ''))
        except Exception as e:
            entry['connect'] = '%s: %s' % (type(e).__name__, e)
            log('  %d: %s' % (port, entry['connect']))
        out.append(entry)
    return out


def main():
    p = dp.base_parser(__doc__.splitlines()[0])
    p.add_argument('--max', type=int, default=3, help='connections to open per port (default %(default)s)')
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument('--driver-running', dest='driver', action='store_const', const='running')
    g.add_argument('--driver-stopped', dest='driver', action='store_const', const='stopped')
    p.add_argument('--probe-ports', action='store_true')
    args = p.parse_args()
    s = dp.Session(args.out, 't2_connections_driver-%s' % args.driver, args)
    s.log('ROS driver: %s' % args.driver)

    summary = {'driver': args.driver,
               'dashboard': test_dashboard(args, s.log),
               'realtime': test_realtime(args, s.log)}
    if args.probe_ports:
        summary['other_ports'] = probe_ports(args, s.log)
    dp.write_json(os.path.join(s.dir, 'summary.json'), summary)


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""T1: realtime feed (port 30004) timing. No motion.

Records the feed for --duration seconds and reports:
  - packet interval as seen by this computer (arrival times)
  - packet interval by the controller's own clock (controller_timer)
  - gaps, bunching (packets arriving together), and clock drift

Answers: is the feed a steady 8 ms, and can a control loop be paced by it?
"""
import os
import time

import dobot_proto as dp


def main():
    p = dp.base_parser(__doc__.splitlines()[0])
    p.add_argument('--duration', type=float, default=60.0, help='seconds (default %(default)s)')
    args = p.parse_args()
    s = dp.Session(args.out, 't1_realtime_feed', args)

    rt = dp.RealtimeReader(args.ip)
    rt.start()
    rt.wait_first()
    s.log('recording %.0f s of realtime packets...' % args.duration)
    t_end = dp.now() + args.duration
    while dp.now() < t_end:
        if rt.error:
            break
        time.sleep(0.2)
    rt.stop()
    if rt.error:
        s.log('feed error: %s' % rt.error)

    packets = rt.slice(0)
    dp.write_realtime_csv(os.path.join(s.dir, 'realtime.csv'), packets)
    if len(packets) < 2:
        s.log('not enough packets (%d)' % len(packets))
        return

    host_t = [t for t, _ in packets]
    ctrl_t = [pk['controller_timer'] for _, pk in packets]
    host_dt = [(b - a) * 1e3 for a, b in zip(host_t, host_t[1:])]
    ctrl_dt = [float(b - a) for a, b in zip(ctrl_t, ctrl_t[1:])]
    span = host_t[-1] - host_t[0]
    ctrl_span_s = (ctrl_t[-1] - ctrl_t[0]) / 1e3
    drift_ms_per_min = ((ctrl_span_s - span) / span * 60e3) if span > 0 else float('nan')
    nominal = dp.percentile(sorted(ctrl_dt), 50)

    summary = {
        'packets': len(packets),
        'unframed': rt.unframed,
        'error': str(rt.error) if rt.error else None,
        'duration_s': span,
        'rate_hz': (len(packets) - 1) / span if span > 0 else None,
        'host_interval_ms': dp.stats(host_dt),
        'controller_interval_ms': dp.stats(ctrl_dt),
        'controller_steps_seen_ms': sorted({round(v, 3) for v in ctrl_dt})[:20],
        'host_gaps_over_12ms': sum(1 for v in host_dt if v > 12.0),
        'host_gaps_over_20ms': sum(1 for v in host_dt if v > 20.0),
        'host_bunched_under_2ms': sum(1 for v in host_dt if v < 2.0),
        'controller_skips': sum(1 for v in ctrl_dt if nominal and v > 1.5 * nominal),
        'controller_clock_drift_ms_per_min': drift_ms_per_min,
        'robot_modes_seen': sorted({dp.mode_name(pk['robot_mode']) for _, pk in packets}),
    }
    dp.write_json(os.path.join(s.dir, 'summary.json'), summary)

    s.log('packets: %d in %.1f s  (%.1f Hz), out of frame: %d'
          % (summary['packets'], span, summary['rate_hz'], rt.unframed))
    s.log('interval, arrival on this computer: %s' % dp.fmt_stats(summary['host_interval_ms']))
    s.log('interval, controller_timer:        %s' % dp.fmt_stats(summary['controller_interval_ms']))
    s.log('controller_timer steps seen (ms): %s' % summary['controller_steps_seen_ms'])
    s.log('arrival gaps > 12 ms: %d, > 20 ms: %d; bunched (< 2 ms apart): %d'
          % (summary['host_gaps_over_12ms'], summary['host_gaps_over_20ms'],
             summary['host_bunched_under_2ms']))
    s.log('controller skipped cycles: %d' % summary['controller_skips'])
    s.log('controller clock vs this computer: %+.2f ms per minute' % drift_ms_per_min)
    s.log('robot modes seen: %s' % ', '.join(summary['robot_modes_seen']))


if __name__ == '__main__':
    main()

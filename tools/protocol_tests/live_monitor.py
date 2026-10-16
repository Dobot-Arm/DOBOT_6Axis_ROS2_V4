#!/usr/bin/env python3
"""Live monitor of the arm's motion from the realtime feed (port 30004). No motion.

Read-only, so it can run next to the ROS driver (30004 serves several clients).
While the arm moves it prints a line a second:

    time      mode     TCP tgt/act mm/s   ripple tgt/act     hitches  longest
    01:20:40  RUNNING  152/150  max 160   4.1%/ 6.0%  9/12   0        24.6 ms

  TCP tgt/act   mean target and actual tool speed over the second, and the peak
  ripple        target and actual tool speed's deviation from its own 150 ms
                moving average, 3rd-97th percentile spread: in % of the mean
                speed and in mm/s. Only where the speed is above 30 % of the
                second's peak, so starts and stops do not count.
  hitches       ServoJ commands taken up more than --hitch-ms after the
                previous one (current_command_id), i.e. late commands
  longest       the longest such gap in the second

and a summary line when a move ends. Alarms (ERROR, COLLISION) are printed at
once.

    python3 live_monitor.py                       # live, controller 192.168.5.1
    python3 live_monitor.py --record rec.csv      # also save every packet
    python3 live_monitor.py --replay realtime.csv # a recorded feed instead
"""
import argparse
import csv
import socket
import time
from collections import deque

import numpy as np

import dobot_proto as dp

MOVING_MM_S = 5.0          # tool target speed above this: the arm is moving
STOPPED_FOR_S = 0.3        # below it this long: the move has ended
SMOOTH_S = 0.15            # moving average the ripple is measured against
STREAM_GAP_S = 0.1         # command ids changing at least this often: a stream


def tcp_speed(arr):
    return float(np.linalg.norm(arr[:3]))


def packets_live(ip, record):
    sock = socket.create_connection((ip, dp.RT_PORT), timeout=3.0)
    sock.settimeout(2.0)
    buf = bytearray(dp.PACKET_SIZE)
    view = memoryview(buf)
    writer = None
    if record:
        f = open(record, 'w', newline='')
        writer = csv.writer(f)
        writer.writerow(dp.REALTIME_CSV_HEADER)
    while True:
        got = 0
        while got < dp.PACKET_SIZE:
            n = sock.recv_into(view[got:], dp.PACKET_SIZE - got)
            if n == 0:
                raise ConnectionError('realtime port closed the connection')
            got += n
        t = dp.now()
        pkt = dp.decode_packet(buf)
        if not dp.packet_is_framed(pkt):
            raise ConnectionError('realtime packet out of frame')
        if writer:
            writer.writerow(dp.realtime_row(t, pkt))
        yield time.time(), t, pkt


def packets_replay(path):
    with open(path) as f:
        for r in csv.DictReader(f):
            def six(name):
                return [float(r['%s_%d' % (name, i)]) for i in range(1, 7)]
            pkt = {'robot_mode': int(float(r['robot_mode'])),
                   'current_command_id': int(float(r['current_command_id'])),
                   'error_status': int(float(r['error_status'])),
                   'collision_state': int(float(r['collision_state'])),
                   'tcp_speed_target': six('tcp_speed_target'),
                   'tcp_speed_actual': six('tcp_speed_actual')}
            t = float(r['host_t'])
            yield t, t, pkt     # wall = the recording's own clock; printed relative


def spread(dev, speed, peak):
    """3rd-97th percentile spread of dev where speed is above 30 % of peak:
    in % of the mean speed there, and in mm/s."""
    keep = speed > 0.3 * peak
    if keep.sum() < 10:
        return None, None
    d = dev[keep]
    mm = float(np.percentile(d, 97) - np.percentile(d, 3))
    return 100.0 * mm / float(np.mean(speed[keep])), mm


class Monitor:
    def __init__(self, hitch_ms):
        self.hitch_s = hitch_ms / 1e3
        self.win = deque()                   # (t, target, actual) over the last ~1.5 s
        self.sec = []                        # this second's samples
        self.sec_start = None
        self.last_id = None
        self.last_change = None
        self.sec_hitches = []
        self.move = None
        self.still_since = None
        self.last_mode = None

    def ripple(self, t0):
        """Ripple of the samples since t0, against a centred moving average
        computed from the window (which extends before t0)."""
        t = np.array([s[0] for s in self.win])
        tgt = np.array([s[1] for s in self.win])
        act = np.array([s[2] for s in self.win])
        n = max(3, int(round(SMOOTH_S / max(np.median(np.diff(t)), 1e-3))) | 1)
        pad = n // 2

        def dev(x):
            avg = np.convolve(np.pad(x, pad, mode='edge'), np.ones(n) / n, mode='valid')
            return x - avg
        # the last pad samples have no future yet: leave them for the next second
        sel = (t >= t0) & (np.arange(len(t)) < len(t) - pad)
        peak = self.move['peak_t'] if self.move else (tgt[sel].max() if sel.any() else 0.0)
        rt = spread(dev(tgt)[sel], tgt[sel], peak)
        ra = spread(dev(act)[sel], act[sel], peak)
        return rt, ra, sel.sum()

    def feed(self, wall, t, pkt, out):
        mode = pkt['robot_mode']
        tgt = tcp_speed(pkt['tcp_speed_target'])
        act = tcp_speed(pkt['tcp_speed_actual'])
        if mode in dp.ERROR_MODES and self.last_mode not in dp.ERROR_MODES:
            out('%s  ALARM: %s (error_status %d, collision %d)'
                % (time.strftime('%H:%M:%S', time.gmtime(wall)), dp.mode_name(mode),
                   pkt['error_status'], pkt['collision_state']))
        self.last_mode = mode

        cid = pkt['current_command_id']
        if self.last_id is not None and cid != self.last_id:
            if self.last_change is not None:
                gap = t - self.last_change
                if self.hitch_s < gap and gap < 1.0 and mode == 7:
                    self.sec_hitches.append(gap)
                    if self.move:
                        self.move['hitches'].append(gap)
            self.last_change = t
        self.last_id = cid

        self.win.append((t, tgt, act))
        while self.win and t - self.win[0][0] > 1.5:
            self.win.popleft()

        moving = tgt > MOVING_MM_S or mode == 7
        if moving:
            self.still_since = None
            if self.move is None:
                self.move = {'start': t, 'wall': wall, 'peak_t': 0.0, 'peak_a': 0.0,
                             'ripple_t': [], 'hitches': []}
                self.sec_start, self.sec = t, []
                self.sec_hitches = []
            self.move['peak_t'] = max(self.move['peak_t'], tgt)
            self.move['peak_a'] = max(self.move['peak_a'], act)
            self.sec.append((tgt, act))
            if t - self.sec_start >= 1.0:
                self.second(wall, mode, out)
        elif self.move is not None:
            if self.still_since is None:
                self.still_since = t
            elif t - self.still_since >= STOPPED_FOR_S:
                if len(self.sec) > 0.3 * 122:        # a remainder worth a line of its own
                    self.second(wall, mode, out)
                m = self.move
                self.move = None
                if m['peak_t'] < MOVING_MM_S:        # RUNNING while applying a setting
                    return
                rip = max(m['ripple_t']) if m['ripple_t'] else float('nan')
                longest = max(m['hitches']) * 1e3 if m['hitches'] else 0.0
                out('          move %.1f s: peak TCP %.0f/%.0f mm/s, ripple up to %.1f%%, '
                    '%d hitch(es)%s'
                    % (t - m['start'] - STOPPED_FOR_S, m['peak_t'], m['peak_a'], rip,
                       len(m['hitches']),
                       ', longest %.0f ms' % longest if m['hitches'] else ''))

    def second(self, wall, mode, out):
        tgt = np.array([s[0] for s in self.sec])
        act = np.array([s[1] for s in self.sec])
        (rt, rt_mm), (ra, ra_mm), _ = self.ripple(self.sec_start)
        if rt is not None and self.move:
            self.move['ripple_t'].append(rt)
        if rt is None and not self.sec_hitches:     # only the stopping ramp left
            self.sec_start, self.sec = self.win[-1][0], []
            return

        def pct(v):
            return '  -  ' if v is None else '%4.1f%%' % v

        def mm(v):
            return ' - ' if v is None else '%2.0f' % v
        longest = max(self.sec_hitches) * 1e3 if self.sec_hitches else 0.0
        out('%s  %-8s %3.0f/%3.0f  max %3.0f   %s/%s %s/%s   %-3d %s'
            % (time.strftime('%H:%M:%S', time.gmtime(wall)), dp.mode_name(mode),
               tgt.mean(), act.mean(), tgt.max(), pct(rt), pct(ra), mm(rt_mm), mm(ra_mm),
               len(self.sec_hitches), '%.1f ms' % longest if self.sec_hitches else ''))
        self.sec_start = self.win[-1][0]
        self.sec = []
        self.sec_hitches = []


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--ip', default=dp.DEFAULT_IP)
    p.add_argument('--hitch-ms', type=float, default=40.0,
                   help='a command taken up later than this after the previous one is a hitch '
                        '(default %(default)s; ServoJ every 20 ms is taken up 16, 25 and now and then 33 ms apart)')
    p.add_argument('--record', help='also write every packet to this CSV')
    p.add_argument('--replay', help='read a recorded realtime.csv instead of the controller')
    args = p.parse_args()
    print('time      mode        TCP tgt/act  peak  ripple tgt/act %  mm/s   hitches longest')
    mon = Monitor(args.hitch_ms)
    source = packets_replay(args.replay) if args.replay else packets_live(args.ip, args.record)
    first = None
    try:
        for wall, t, pkt in source:
            if args.replay:             # time since the start of the recording
                first = wall if first is None else first
                wall = wall - first
            mon.feed(wall, t, pkt, lambda line: print(line, flush=True))
    except KeyboardInterrupt:
        pass


if __name__ == '__main__':
    main()

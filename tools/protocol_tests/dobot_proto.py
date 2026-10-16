"""Shared helpers for the Dobot V4 protocol tests.

Talks to the controller directly, without ROS:
  - port 29999: text commands, each answered by one ';'-terminated reply
                ("ErrorID,{value},Command(...);")
  - port 30004: 1440-byte binary realtime packets, one every ~8 ms

Python 3.8+, standard library only, so it runs on the robot's computer as is.
"""

import argparse
import bisect
import csv
import json
import math
import os
import socket
import struct
import sys
import threading
import time
from collections import deque

DEFAULT_IP = '192.168.5.1'
DASH_PORT = 29999
RT_PORT = 30004
PACKET_SIZE = 1440
TEST_VALUE = 0x0123456789ABCDEF

ROBOT_MODES = {
    1: 'INIT', 2: 'BRAKE_OPEN', 3: 'POWER_OFF', 4: 'DISABLED', 5: 'ENABLE',
    6: 'BACKDRIVE', 7: 'RUNNING', 8: 'SINGLE_MOVE', 9: 'ERROR', 10: 'PAUSE',
    11: 'COLLISION',
}
ERROR_MODES = (9, 11)


def now():
    return time.monotonic()


def mode_name(mode):
    return '%s(%s)' % (ROBOT_MODES.get(mode, '?'), mode)


# ---------------------------------------------------------------------------
# Realtime packet (port 30004). Offsets from RealTimeData in
# dobot_bringup_v4/include/dobot_bringup/command.h. Little endian.
# ---------------------------------------------------------------------------

_SCALARS = [
    ('len', '<H', 0),
    ('robot_mode', '<Q', 24),
    ('controller_timer', '<Q', 32),
    ('run_time', '<Q', 40),
    ('test_value', '<Q', 48),
    ('speed_scaling', '<d', 64),
    ('velocity_ratio', '<b', 1016),
    ('enable_status', '<b', 1026),
    ('running_status', '<b', 1028),
    ('error_status', '<b', 1029),
    ('collision_state', '<b', 1038),
    ('current_command_id', '<Q', 1112),
]
_ARRAYS = [
    ('q_target', 192),
    ('qd_target', 240),
    ('qdd_target', 288),
    ('q_actual', 432),
    ('qd_actual', 480),
    ('i_actual', 528),
    ('tcp_speed_actual', 672),
    ('tcp_speed_target', 816),
    ('m_actual', 1120),
]
_SCALAR_STRUCTS = [(n, struct.Struct(f), o) for n, f, o in _SCALARS]
_ARRAY6 = struct.Struct('<6d')

CSV_SCALARS = ['controller_timer', 'robot_mode', 'speed_scaling', 'velocity_ratio',
               'enable_status', 'running_status', 'error_status', 'collision_state',
               'current_command_id']
CSV_ARRAYS = [n for n, _ in _ARRAYS]
REALTIME_CSV_HEADER = (['host_t'] + CSV_SCALARS +
                       ['%s_%d' % (n, i + 1) for n in CSV_ARRAYS for i in range(6)])


def decode_packet(buf):
    pkt = {}
    for name, st, off in _SCALAR_STRUCTS:
        pkt[name] = st.unpack_from(buf, off)[0]
    for name, off in _ARRAYS:
        pkt[name] = _ARRAY6.unpack_from(buf, off)
    return pkt


def packet_is_framed(pkt):
    return pkt['len'] == PACKET_SIZE and pkt['test_value'] == TEST_VALUE


def realtime_row(t, pkt):
    row = [t] + [pkt[n] for n in CSV_SCALARS]
    for n in CSV_ARRAYS:
        row.extend(pkt[n])
    return row


class StaleRealtime(RuntimeError):
    pass


class RealtimeReader(threading.Thread):
    """Reads port 30004 in the background and keeps every decoded packet.

    Packets are kept in memory (about 125 per second), so each test can slice
    out the packets that belong to one run and write them next to it.
    """

    def __init__(self, ip, port=RT_PORT, keep=True, timeout=1.0):
        super().__init__(daemon=True)
        self.ip, self.port, self.keep = ip, port, keep
        self.sock = socket.create_connection((ip, port), timeout=3.0)
        self.sock.settimeout(timeout)
        self.lock = threading.Lock()
        self.packets = []          # [(host_t, pkt)]
        self.latest = None         # (host_t, pkt)
        self.count = 0
        self.unframed = 0
        self.error = None
        self._stop_evt = threading.Event()

    def _recv_exact(self, view):
        got = 0
        while got < len(view):
            n = self.sock.recv_into(view[got:], len(view) - got)
            if n == 0:
                raise ConnectionError('realtime port closed the connection')
            got += n

    def run(self):
        buf = bytearray(PACKET_SIZE)
        view = memoryview(buf)
        try:
            while not self._stop_evt.is_set():
                try:
                    self._recv_exact(view)
                except socket.timeout:
                    raise ConnectionError('no realtime data for %.1f s' % self.sock.gettimeout())
                t = now()
                pkt = decode_packet(buf)
                if not packet_is_framed(pkt):
                    # Out of frame: the stream can't be trusted from here on.
                    self.unframed += 1
                    raise ConnectionError('realtime packet out of frame (len=%d, test_value=0x%x)'
                                          % (pkt['len'], pkt['test_value']))
                with self.lock:
                    self.latest = (t, pkt)
                    self.count += 1
                    if self.keep:
                        self.packets.append((t, pkt))
        except Exception as e:  # reported to the main thread via .error
            if not self._stop_evt.is_set():
                self.error = e
        finally:
            try:
                self.sock.close()
            except OSError:
                pass

    def stop(self):
        self._stop_evt.set()
        try:
            self.sock.shutdown(socket.SHUT_RDWR)
        except OSError:
            pass
        self.join(timeout=2.0)

    def wait_first(self, timeout=3.0):
        deadline = now() + timeout
        while now() < deadline:
            if self.error:
                raise self.error
            if self.latest:
                return self.latest[1]
            time.sleep(0.01)
        raise StaleRealtime('no realtime packet within %.1f s' % timeout)

    def current(self, max_age=0.1):
        if self.error:
            raise self.error
        latest = self.latest
        if latest is None or now() - latest[0] > max_age:
            raise StaleRealtime('realtime data older than %.0f ms' % (max_age * 1e3))
        return latest[1]

    def index(self):
        with self.lock:
            return len(self.packets)

    def slice(self, start, end=None):
        with self.lock:
            return list(self.packets[start:end])


def write_realtime_csv(path, packets):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(REALTIME_CSV_HEADER)
        for t, pkt in packets:
            w.writerow(realtime_row(t, pkt))


# ---------------------------------------------------------------------------
# Command port (29999)
# ---------------------------------------------------------------------------

class Reply:
    __slots__ = ('raw', 'err', 'value', 't_send', 't_recv')

    def __init__(self, raw, t_send, t_recv):
        self.raw, self.t_send, self.t_recv = raw, t_send, t_recv
        self.err, self.value = parse_reply(raw)

    @property
    def rtt_ms(self):
        return (self.t_recv - self.t_send) * 1e3

    @property
    def echo(self):
        """The command name the controller echoes back, e.g. 'RobotMode'."""
        tail = self.raw.rsplit('},', 1)[-1] if '},' in self.raw else self.raw
        return tail.split('(', 1)[0].strip()

    def __repr__(self):
        return 'Reply(%r, %.2f ms)' % (self.raw, self.rtt_ms)


def parse_reply(raw):
    head = raw.split(',', 1)[0].strip()
    try:
        err = int(head)
    except ValueError:
        err = None
    lb, rb = raw.find('{'), raw.rfind('}')
    value = raw[lb + 1:rb] if lb != -1 and rb > lb else ''
    return err, value


class Dashboard:
    """One connection to port 29999.

    send() and recv_reply() are separate so a test can pipeline: send several
    commands, then read the replies. Replies are split on ';' and buffered, so
    two replies arriving in one TCP segment are handled.
    """

    def __init__(self, ip, port=DASH_PORT, timeout=2.0, nodelay=True):
        self.sock = socket.create_connection((ip, port), timeout=3.0)
        if nodelay:
            self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        self.sock.settimeout(timeout)
        self._buf = b''
        self._buf_t = 0.0

    def close(self):
        try:
            self.sock.close()
        except OSError:
            pass

    def send(self, cmd):
        t = now()
        self.sock.sendall(cmd.encode())
        return t

    def recv_reply(self):
        """Return (raw_reply, arrival_time). Arrival time is when the TCP chunk
        that completed this reply was received."""
        while b';' not in self._buf:
            chunk = self.sock.recv(4096)
            if not chunk:
                raise ConnectionError('command port closed the connection')
            self._buf += chunk
            self._buf_t = now()
        raw, _, self._buf = self._buf.partition(b';')
        return raw.decode(errors='replace') + ';', self._buf_t

    def query(self, cmd):
        t_send = self.send(cmd)
        raw, t_recv = self.recv_reply()
        return Reply(raw, t_send, t_recv)


def servoj_cmd(target, servo_t, aheadtime, gain):
    return ('ServoJ(%.4f,%.4f,%.4f,%.4f,%.4f,%.4f,t=%.4f,aheadtime=%g,gain=%g)'
            % (tuple(target) + (servo_t, aheadtime, gain)))


# ---------------------------------------------------------------------------
# Statistics and small numeric helpers
# ---------------------------------------------------------------------------

def percentile(sorted_vals, p):
    if not sorted_vals:
        return float('nan')
    k = (len(sorted_vals) - 1) * p / 100.0
    lo, hi = math.floor(k), math.ceil(k)
    return sorted_vals[lo] + (sorted_vals[hi] - sorted_vals[lo]) * (k - lo)


def stats(values):
    vals = sorted(v for v in values if v is not None and not math.isnan(v))
    if not vals:
        return {'n': 0}
    return {
        'n': len(vals),
        'min': vals[0],
        'median': percentile(vals, 50),
        'p90': percentile(vals, 90),
        'p99': percentile(vals, 99),
        'max': vals[-1],
        'mean': sum(vals) / len(vals),
    }


def fmt_stats(s, unit='ms'):
    if not s.get('n'):
        return 'n=0'
    return ('n=%d  min %.2f  median %.2f  p90 %.2f  p99 %.2f  max %.2f %s'
            % (s['n'], s['min'], s['median'], s['p90'], s['p99'], s['max'], unit))


def interp(xs, ys, x):
    """Linear interpolation on sorted xs; None outside the range."""
    if not xs or x < xs[0] or x > xs[-1]:
        return None
    i = bisect.bisect_right(xs, x)
    if i >= len(xs):
        return ys[-1]
    x0, x1 = xs[i - 1], xs[i]
    if x1 == x0:
        return ys[i]
    return ys[i - 1] + (ys[i] - ys[i - 1]) * (x - x0) / (x1 - x0)


def estimate_lag(cmd_t, cmd_v, sig_t, sig_v, max_lag=0.4, step=0.002):
    """Delay (s) that best aligns sig with cmd: sig(t) ~ cmd(t - lag).

    Returns (lag_s, rms, max_over, max_under) where max_over/under are the
    largest deviations of sig above/below the shifted command.
    """
    best = None
    n_steps = int(max_lag / step) + 1
    for i in range(n_steps):
        lag = i * step
        sq, n = 0.0, 0
        for t, v in zip(sig_t, sig_v):
            c = interp(cmd_t, cmd_v, t - lag)
            if c is None:
                continue
            sq += (v - c) ** 2
            n += 1
        if n < 10:
            continue
        rms = math.sqrt(sq / n)
        if best is None or rms < best[1]:
            best = (lag, rms)
    if best is None:
        return None
    lag, rms = best
    devs = []
    for t, v in zip(sig_t, sig_v):
        c = interp(cmd_t, cmd_v, t - lag)
        if c is not None:
            devs.append(v - c)
    return lag, rms, max(devs), min(devs)


# ---------------------------------------------------------------------------
# Motion profiles. Each returns f(t) -> 6 joint targets (deg), plus duration.
# ---------------------------------------------------------------------------

def hold_profile(q0):
    q0 = list(q0)
    return lambda t: list(q0)


def cosine_profile(q0, joint, amplitude, freq):
    """q0 + A*(1 - cos(2*pi*f*t))/2 on one joint: starts at q0 with zero
    velocity, peaks at q0 + A, back to q0 after every whole cycle.
    Peak speed = pi*A*f deg/s."""
    q0 = list(q0)
    j = joint - 1

    def f(t):
        q = list(q0)
        q[j] += amplitude * (1.0 - math.cos(2.0 * math.pi * freq * t)) / 2.0
        return q
    return f


def trapezoid_1d(distance, vmax, amax):
    """Position along a trapezoidal velocity profile from 0 to `distance`.
    Returns (f(t), duration)."""
    d = abs(distance)
    sign = 1.0 if distance >= 0 else -1.0
    if d * amax < vmax * vmax:      # never reaches vmax: triangular
        vpeak = math.sqrt(d * amax)
        t_acc, t_cruise = vpeak / amax, 0.0
    else:
        vpeak = vmax
        t_acc = vmax / amax
        t_cruise = (d - vmax * t_acc) / vmax
    total = 2 * t_acc + t_cruise

    def f(t):
        if t <= 0:
            return 0.0
        if t >= total:
            return sign * d
        if t < t_acc:
            return sign * 0.5 * amax * t * t
        if t < t_acc + t_cruise:
            return sign * (0.5 * amax * t_acc ** 2 + vpeak * (t - t_acc))
        td = total - t
        return sign * (d - 0.5 * amax * td * td)
    return f, total


def out_and_back_profile(q0, joint, distance, vmax, amax, pause=0.3):
    seg, t_seg = trapezoid_1d(distance, vmax, amax)
    q0 = list(q0)
    j = joint - 1
    duration = 2 * t_seg + pause

    def f(t):
        q = list(q0)
        if t <= t_seg:
            q[j] += seg(t)
        elif t <= t_seg + pause:
            q[j] += seg(t_seg)
        else:
            q[j] += seg(t_seg) - seg(t - t_seg - pause)
        return q
    return f, duration


def check_profile(profile, duration, period, q0, max_excursion, max_speed):
    """Sample the profile and refuse it if it leaves the allowed envelope.
    Returns (max_excursion_per_joint, max_speed_per_joint)."""
    exc = [0.0] * 6
    spd = [0.0] * 6
    prev = profile(0.0)
    n = int(duration / period) + 1
    for k in range(n + 1):
        q = profile(min(k * period, duration))
        for j in range(6):
            exc[j] = max(exc[j], abs(q[j] - q0[j]))
            if k:
                spd[j] = max(spd[j], abs(q[j] - prev[j]) / period)
        prev = q
    first = profile(0.0)
    start_offset = max(abs(first[j] - q0[j]) for j in range(6))
    if max(exc) > max_excursion + 1e-9:
        raise ValueError('profile moves a joint %.2f deg from the start (limit %.2f, '
                         'raise with --max-excursion)' % (max(exc), max_excursion))
    if max(spd) > max_speed + 1e-9:
        raise ValueError('profile commands %.1f deg/s (limit %.1f, raise with --max-speed)'
                         % (max(spd), max_speed))
    return exc, spd, start_offset


# ---------------------------------------------------------------------------
# ServoJ streaming
# ---------------------------------------------------------------------------

class StreamConfig:
    def __init__(self, rate=50.0, servo_t=None, aheadtime=50.0, gain=500.0,
                 timing='index', pipelined=False, delays=None, stop_at=None,
                 stop_cmd=False):
        self.rate = rate
        self.servo_t = servo_t if servo_t else 1.0 / rate
        self.aheadtime = aheadtime
        self.gain = gain
        # 'index': command k carries the target for k * period, and a late
        #          send does not make the next target jump ahead.
        # 'wallclock': the target is for the wall time of the send, and the
        #          schedule catches up after a late send. This is what the
        #          fork's TrajectoryExecutor does today.
        self.timing = timing
        self.pipelined = pipelined
        self.delays = sorted(delays or [])   # [(trajectory_time_s, extra_ms)]
        self.stop_at = stop_at               # stop sending at this trajectory time
        self.stop_cmd = stop_cmd             # with stop_at: send Stop() instead of going quiet

    def as_dict(self):
        return dict(self.__dict__)


COMMAND_CSV_HEADER = (['seq', 't_send', 't_reply', 'rtt_ms', 'traj_t', 'err']
                      + ['target_%d' % (i + 1) for i in range(6)] + ['reply'])


def run_stream(dash, rt, profile, duration, cfg, log=print):
    """Stream ServoJ for `profile` over `duration` seconds of trajectory time.

    Aborts on: a ServoJ error reply, robot mode ERROR/COLLISION, stale
    realtime data, or Ctrl-C. On abort it sends Stop() and records
    GetErrorID(). Returns a result dict with the command rows.
    """
    period = 1.0 / cfg.rate
    rows = []
    pending = deque()
    state = {'abort': None, 'done': False}
    reply_lock = threading.Lock()

    def fill(row, raw, t_recv):
        r = Reply(raw, row['t_send'], t_recv)
        row['t_reply'] = t_recv
        row['rtt_ms'] = r.rtt_ms
        row['err'] = r.err
        row['reply'] = raw
        if r.err != 0 and state['abort'] is None:
            state['abort'] = 'ServoJ rejected: %s' % raw

    def reply_reader():
        while True:
            with reply_lock:
                if state['done'] and not pending:
                    return
            if not pending:
                time.sleep(0.0005)
                continue
            try:
                raw, t_recv = dash.recv_reply()
            except socket.timeout:
                if state['abort'] is None:
                    state['abort'] = 'no reply to ServoJ within the socket timeout'
                return
            except Exception as e:
                if state['abort'] is None:
                    state['abort'] = 'reply reader: %s' % e
                return
            with reply_lock:
                seq = pending.popleft()
                row = rows[seq]
            fill(row, raw, t_recv)

    reader = None
    if cfg.pipelined:
        reader = threading.Thread(target=reply_reader, daemon=True)
        reader.start()

    delays = list(cfg.delays)
    start = now()
    next_t = start
    k = 0
    stopped_early = False
    try:
        while True:
            # Wait for this command's slot.
            delay = next_t - now()
            if delay > 0:
                time.sleep(delay)

            nominal = k * period if cfg.timing == 'index' else now() - start
            # Injected late send (test 7): hold this command back.
            while delays and delays[0][0] <= nominal:
                extra_ms = delays.pop(0)[1]
                time.sleep(extra_ms / 1e3)
            traj_t = k * period if cfg.timing == 'index' else now() - start

            if cfg.stop_at is not None and traj_t >= cfg.stop_at:
                stopped_early = True
                if cfg.stop_cmd:
                    log('    stop_at=%.3f s: sending Stop()' % traj_t)
                    break
                log('    stop_at=%.3f s: going quiet' % traj_t)
                break

            last = traj_t >= duration
            if last:
                traj_t = duration

            if state['abort']:
                break
            pkt = rt.current(max_age=0.1)
            if pkt['robot_mode'] in ERROR_MODES:
                state['abort'] = 'robot mode %s' % mode_name(pkt['robot_mode'])
                break

            target = profile(traj_t)
            cmd = servoj_cmd(target, cfg.servo_t, cfg.aheadtime, cfg.gain)
            row = {'seq': k, 'traj_t': traj_t, 'target': target, 't_reply': None,
                   'rtt_ms': None, 'err': None, 'reply': ''}
            if cfg.pipelined:
                # Hold the lock across the send so the reader can't match a
                # reply to this row before its send time is recorded.
                with reply_lock:
                    rows.append(row)
                    pending.append(k)
                    row['t_send'] = dash.send(cmd)
            else:
                rows.append(row)
                row['t_send'] = dash.send(cmd)
                raw, t_recv = dash.recv_reply()
                fill(row, raw, t_recv)
                if state['abort']:
                    break
            k += 1
            if last:
                break

            if cfg.timing == 'index':
                if row['t_send'] - next_t > period / 2:
                    next_t = row['t_send'] + period   # late: restart the schedule, no burst
                else:
                    next_t += period
            else:
                next_t = start + k * period  # late: catch up, as the fork does
    except KeyboardInterrupt:
        state['abort'] = 'interrupted (Ctrl-C)'
    except StaleRealtime as e:
        state['abort'] = str(e)
    except Exception as e:
        state['abort'] = '%s: %s' % (type(e).__name__, e)
    finally:
        with reply_lock:
            state['done'] = True
        if reader:
            reader.join(timeout=3.0)

    t_end = now()
    result = {
        'config': cfg.as_dict(),
        'rows': rows,
        'abort': state['abort'],
        'stopped_early': stopped_early,
        't_start': start,
        't_end': t_end,
        'error_ids': None,
    }
    if state['abort'] or (stopped_early and cfg.stop_cmd):
        if state['abort']:
            log('    ABORT: %s' % state['abort'])
        try:
            r = dash.query('Stop()')
            result['stop_reply'] = r.raw
        except Exception as e:
            result['stop_reply'] = 'failed: %s' % e
    if state['abort']:
        try:
            result['error_ids'] = dash.query('GetErrorID()').raw
            log('    GetErrorID: %s' % result['error_ids'])
        except Exception as e:
            result['error_ids'] = 'failed: %s' % e
    return result


def write_command_csv(path, rows):
    with open(path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(COMMAND_CSV_HEADER)
        for r in rows:
            w.writerow([r['seq'], r['t_send'], r['t_reply'], r['rtt_ms'], r['traj_t'], r['err']]
                       + list(r['target']) + [r['reply']])


def stream_summary(result, packets, joint=None, servo_t=None):
    """Numbers worth printing for one streamed run."""
    rows = result['rows']
    sends = [r['t_send'] for r in rows]
    gaps = [(b - a) * 1e3 for a, b in zip(sends, sends[1:])]
    rtts = [r['rtt_ms'] for r in rows if r['rtt_ms'] is not None]
    errors = [r for r in rows if r['err'] not in (0, None)]
    s = {
        'sent': len(rows),
        'rejected': len(errors),
        'first_rejection': errors[0]['reply'] if errors else None,
        'abort': result['abort'],
        'error_ids': result['error_ids'],
        'send_gap_ms': stats(gaps),
        'rtt_ms': stats(rtts),
    }
    if packets:
        modes = sorted({p['robot_mode'] for _, p in packets})
        s['modes_seen'] = [mode_name(m) for m in modes]
        s['speed_scaling_seen'] = sorted({round(p['speed_scaling'], 4) for _, p in packets})
    if joint and packets and rows:
        j = joint - 1
        st = servo_t or result['config']['servo_t']
        steps = [abs(b['target'][j] - a['target'][j]) / st for a, b in zip(rows, rows[1:])]
        s['joint'] = joint
        s['peak_commanded_step_speed_dps'] = max(steps) if steps else 0.0
        s['peak_qd_target_dps'] = max(abs(p['qd_target'][j]) for _, p in packets)
        s['peak_qd_actual_dps'] = max(abs(p['qd_actual'][j]) for _, p in packets)
        cmd_t = sends
        cmd_v = [r['target'][j] for r in rows]
        for name in ('q_target', 'q_actual'):
            lag = estimate_lag(cmd_t, cmd_v, [t for t, _ in packets],
                               [p[name][j] for _, p in packets])
            if lag:
                s['lag_%s_ms' % name] = round(lag[0] * 1e3, 1)
                s['rms_%s_deg' % name] = round(lag[1], 4)
                s['max_over_%s_deg' % name] = round(lag[2], 4)
                s['max_under_%s_deg' % name] = round(lag[3], 4)
    return s


def print_stream_summary(s, log=print):
    log('    sent %d, rejected %d%s' % (s['sent'], s['rejected'],
                                     ('  first: %s' % s['first_rejection']) if s['rejected'] else ''))
    log('    send gap: %s' % fmt_stats(s['send_gap_ms']))
    log('    reply RTT: %s' % fmt_stats(s['rtt_ms']))
    if 'modes_seen' in s:
        log('    modes seen: %s   speed_scaling: %s' % (', '.join(s['modes_seen']),
                                                      s['speed_scaling_seen']))
    if 'joint' in s:
        log('    J%d peak speed: commanded step %.2f, qd_target %.2f, qd_actual %.2f deg/s'
            % (s['joint'], s['peak_commanded_step_speed_dps'], s['peak_qd_target_dps'],
               s['peak_qd_actual_dps']))
        if 'lag_q_target_ms' in s:
            log('    J%d lag: q_target %.1f ms (rms %.4f deg), q_actual %.1f ms (rms %.4f deg)'
                % (s['joint'], s['lag_q_target_ms'], s['rms_q_target_deg'],
                   s.get('lag_q_actual_ms', float('nan')), s.get('rms_q_actual_deg', float('nan'))))
    if s['abort']:
        log('    aborted: %s' % s['abort'])


# ---------------------------------------------------------------------------
# Session plumbing shared by the test scripts
# ---------------------------------------------------------------------------

def base_parser(description, motion=False):
    p = argparse.ArgumentParser(description=description)
    p.add_argument('--ip', default=DEFAULT_IP, help='controller IP (default %(default)s)')
    p.add_argument('--out', default='results', help='results directory (default %(default)s)')
    if motion:
        p.add_argument('--rate', type=float, default=50.0, help='ServoJ rate, Hz (default %(default)s)')
        p.add_argument('--servo-t', type=float, default=None,
                       help='ServoJ t, s (default 1/rate)')
        p.add_argument('--aheadtime', type=float, default=50.0)
        p.add_argument('--gain', type=float, default=500.0)
        p.add_argument('--pipelined', action='store_true',
                       help='send without waiting for each reply')
        p.add_argument('--max-excursion', type=float, default=15.0,
                       help='refuse profiles that move a joint further than this, deg '
                            '(default %(default)s)')
        p.add_argument('--max-speed', type=float, default=60.0,
                       help='refuse profiles faster than this, deg/s (default %(default)s)')
        p.add_argument('--yes', action='store_true', help='skip the confirmation prompt')
    return p


class Session:
    """A results directory for one script invocation, plus a log file."""

    def __init__(self, out, name, args):
        self.dir = os.path.join(out, '%s_%s' % (time.strftime('%Y%m%d-%H%M%S'), name))
        os.makedirs(self.dir, exist_ok=True)
        self._log = open(os.path.join(self.dir, 'log.txt'), 'w')
        write_json(os.path.join(self.dir, 'args.json'), vars(args))
        self.log('results: %s' % self.dir)

    def log(self, msg=''):
        print(msg)
        self._log.write(msg + '\n')
        self._log.flush()

    def run_dir(self, name):
        d = os.path.join(self.dir, name)
        os.makedirs(d, exist_ok=True)
        return d

    def save_run(self, name, result, packets, summary):
        d = self.run_dir(name)
        write_command_csv(os.path.join(d, 'commands.csv'), result['rows'])
        write_realtime_csv(os.path.join(d, 'realtime.csv'), packets)
        meta = {k: v for k, v in result.items() if k != 'rows'}
        write_json(os.path.join(d, 'result.json'), meta)
        write_json(os.path.join(d, 'summary.json'), summary)


def write_json(path, obj):
    with open(path, 'w') as f:
        json.dump(obj, f, indent=2, default=str)


def connect(args, log=print):
    log('connecting to %s (29999 commands, 30004 realtime)...' % args.ip)
    dash = Dashboard(args.ip)
    rt = RealtimeReader(args.ip)
    rt.start()
    rt.wait_first()
    return dash, rt


def preflight(dash, rt, log=print):
    """The arm must be enabled and idle. Returns q_actual (deg) to start from."""
    reply = dash.query('RobotMode()')
    pkt = rt.current(max_age=0.2)
    log('robot mode: %s (command port), %s (realtime)' % (reply.value, mode_name(pkt['robot_mode'])))
    if pkt['robot_mode'] != 5:
        raise SystemExit('the arm must be in ENABLE(5) mode, idle. It is %s. Enable it on the '
                         'pendant (or run t11_recovery.py) and try again.' % mode_name(pkt['robot_mode']))
    err = dash.query('GetErrorID()')
    if err.err != 0 or any(ch.isdigit() for ch in err.value):
        log('note: GetErrorID reports %s' % err.raw)
    q = list(pkt['q_actual'])
    log('q_actual: [%s] deg' % ', '.join('%.3f' % v for v in q))
    log('speed_scaling: %s   velocity_ratio: %s' % (pkt['speed_scaling'], pkt['velocity_ratio']))
    return q


def idle_q(rt):
    """q_actual (deg) when the arm is idle in ENABLE(5); raises otherwise."""
    pkt = rt.current(max_age=0.2)
    if pkt['robot_mode'] != 5:
        raise RuntimeError('arm is %s, not ENABLE(5)' % mode_name(pkt['robot_mode']))
    return list(pkt['q_actual'])


def stream_config(args, **overrides):
    kw = dict(rate=args.rate, servo_t=args.servo_t, aheadtime=args.aheadtime,
              gain=args.gain, pipelined=args.pipelined)
    kw.update(overrides)
    return StreamConfig(**kw)


def confirm(lines, yes, log=print):
    for line in lines:
        log(line)
    if yes:
        return
    if not sys.stdin.isatty():
        raise SystemExit('not a terminal: pass --yes to run without the prompt')
    answer = input("Clear workspace, e-stop in hand? Type 'yes' to move the arm: ")
    if answer.strip().lower() != 'yes':
        raise SystemExit('cancelled')


def wait_idle(dash, rt, timeout=5.0, log=print):
    """After a stream, wait for ENABLE(5). Sends Stop() if it doesn't come."""
    deadline = now() + timeout
    while now() < deadline:
        mode = rt.current(max_age=0.5)['robot_mode']
        if mode == 5:
            return mode
        if mode in ERROR_MODES:
            return mode
        time.sleep(0.02)
    log('    not idle after %.1f s (mode %s): sending Stop()' % (timeout, mode_name(mode)))
    dash.query('Stop()')
    deadline = now() + timeout
    while now() < deadline:
        mode = rt.current(max_age=0.5)['robot_mode']
        if mode == 5 or mode in ERROR_MODES:
            return mode
        time.sleep(0.02)
    return mode


def do_run(sess, dash, rt, name, profile, duration, cfg, joint=None, observe=0.0, tail=0.5):
    """Stream one run, record everything around it, print and save a summary.

    observe: seconds to keep recording right after the last command (before
             waiting for idle), e.g. to watch the arm after the stream stops.
    tail:    seconds recorded after the arm is idle again.
    """
    sess.log('  run %s' % name)
    i0 = rt.index()
    result = run_stream(dash, rt, profile, duration, cfg, log=sess.log)
    if observe:
        time.sleep(observe)
    if result['abort']:
        try:
            final_mode = rt.current(max_age=0.5)['robot_mode']
        except Exception:
            final_mode = None
    else:
        final_mode = wait_idle(dash, rt, log=sess.log)
    time.sleep(tail)
    packets = rt.slice(i0)
    summary = stream_summary(result, packets, joint=joint)
    summary['final_mode'] = mode_name(final_mode) if final_mode is not None else None
    print_stream_summary(summary, log=sess.log)
    sess.log('    final mode: %s' % summary['final_mode'])
    sess.save_run(name, result, packets, summary)
    return summary, result, packets


def parse_list(text, cast=float):
    return [cast(x) for x in text.split(',') if x.strip()]

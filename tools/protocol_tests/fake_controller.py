#!/usr/bin/env python3
"""A crude stand-in for the controller, for checking the test scripts.

Serves port 29999 (text commands) and 30004 (1440-byte packets every 8 ms)
on localhost. ServoJ targets are followed through a first-order lag. It is
NOT a model of the real controller's filter; it only exercises the scripts'
plumbing, including the abort paths:

    --reject-speed V   reply -2 and go to ERROR(9) when a ServoJ step implies
                       more than V deg/s (step / t)

Run it, then point any test at it with --ip 127.0.0.1.
"""
import argparse
import math
import re
import socket
import struct
import threading
import time

import dobot_proto as dp


class Robot:
    def __init__(self, reject_speed):
        self.lock = threading.Lock()
        self.mode = 5
        self.q = [0.0, 10.0, 90.0, 0.0, -90.0, 0.0]
        self.q_target = list(self.q)
        self.qd = [0.0] * 6
        self.cmd = list(self.q)
        self.last_servo = 0.0
        self.speed = 1.0
        self.reject_speed = reject_speed
        self.error = False

    def step(self, dt):
        with self.lock:
            if self.mode == 7 and time.monotonic() - self.last_servo > 0.2:
                self.mode = 5
            alpha = 1.0 - math.exp(-dt / 0.03)
            for j in range(6):
                prev = self.q[j]
                self.q_target[j] += (self.cmd[j] - self.q_target[j]) * min(1.0, dt / 0.02)
                self.q[j] += (self.q_target[j] - self.q[j]) * alpha
                self.qd[j] = (self.q[j] - prev) / dt

    def handle(self, cmd):
        name = cmd.split('(', 1)[0].strip()
        with self.lock:
            if name == 'RobotMode':
                return '0,{%d},RobotMode();' % self.mode
            if name == 'GetErrorID':
                ids = '[[22]]' if self.error else '[[],[],[],[],[],[],[]]'
                return '0,{%s},GetErrorID();' % ids
            if name == 'GetAngle':
                return '0,{%s},GetAngle();' % ','.join('%.4f' % v for v in self.q)
            if name == 'GetPose':
                return '0,{300.0,0.0,400.0,180.0,0.0,0.0},GetPose();'
            if name == 'Stop':
                if self.mode == 7:
                    self.mode = 5
                self.cmd = list(self.q)
                return '0,{},Stop();'
            if name == 'ClearError':
                self.error = False
                if self.mode == 9:
                    self.mode = 5
                return '0,{},ClearError();'
            if name == 'EnableRobot':
                self.mode = 5
                return '0,{},EnableRobot();'
            if name == 'SpeedFactor':
                self.speed = float(re.findall(r'[\d.]+', cmd)[0]) / 100.0
                return '0,{},SpeedFactor(%s);' % cmd.split('(', 1)[1].rstrip(')')
            if name == 'ServoJ':
                if self.mode == 9:
                    return '-2,{},%s;' % cmd
                args = cmd[cmd.index('(') + 1:cmd.rindex(')')].split(',')
                target = [float(a) for a in args[:6]]
                t = float([a for a in args if a.strip().startswith('t=')][0].split('=')[1])
                speed = max(abs(a - b) for a, b in zip(target, self.cmd)) / t
                if self.reject_speed and speed > self.reject_speed:
                    self.mode, self.error = 9, True
                    return '-2,{},%s;' % cmd
                self.cmd = target
                self.mode = 7
                self.last_servo = time.monotonic()
                return '0,{},%s;' % cmd
            return '0,{},%s;' % cmd

    def packet(self):
        buf = bytearray(dp.PACKET_SIZE)
        now_ms = int(time.time() * 1000)
        with self.lock:
            struct.pack_into('<H', buf, 0, dp.PACKET_SIZE)
            struct.pack_into('<Q', buf, 24, self.mode)
            struct.pack_into('<Q', buf, 32, now_ms)
            struct.pack_into('<Q', buf, 48, dp.TEST_VALUE)
            struct.pack_into('<d', buf, 64, self.speed)
            struct.pack_into('<b', buf, 1029, 1 if self.error else 0)
            struct.pack_into('<6d', buf, 192, *self.q_target)
            struct.pack_into('<6d', buf, 240, *self.qd)
            struct.pack_into('<6d', buf, 432, *self.q)
            struct.pack_into('<6d', buf, 480, *self.qd)
        return bytes(buf)


def serve_dash(robot, conn):
    buf = b''
    with conn:
        while True:
            data = conn.recv(4096)
            if not data:
                return
            buf += data
            # Commands are not terminated; split on the closing ')'.
            while b')' in buf:
                cmd, _, buf = buf.partition(b')')
                reply = robot.handle(cmd.decode() + ')')
                time.sleep(0.0005)
                conn.sendall(reply.encode())


def serve_rt(robot, conn):
    with conn:
        try:
            while True:
                conn.sendall(robot.packet())
                time.sleep(0.008)
        except OSError:
            return


def listen(port, handler, robot):
    srv = socket.socket()
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(('127.0.0.1', port))
    srv.listen(8)
    while True:
        conn, _ = srv.accept()
        conn.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        threading.Thread(target=handler, args=(robot, conn), daemon=True).start()


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('--reject-speed', type=float, default=0.0)
    args = p.parse_args()
    robot = Robot(args.reject_speed)
    threading.Thread(target=listen, args=(dp.DASH_PORT, serve_dash, robot), daemon=True).start()
    threading.Thread(target=listen, args=(dp.RT_PORT, serve_rt, robot), daemon=True).start()
    print('fake controller on 127.0.0.1:%d and :%d' % (dp.DASH_PORT, dp.RT_PORT))
    last = time.monotonic()
    while True:
        time.sleep(0.002)
        t = time.monotonic()
        robot.step(t - last)
        last = t


if __name__ == '__main__':
    main()

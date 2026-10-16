#!/usr/bin/env python3
"""Plot one streamed run: commanded vs controller target vs actual.

    python3 analyze.py results/<session>/<run> [--joint 6] [--show]

Reads commands.csv and realtime.csv from the run directory and writes
plot_J<n>.png next to them. Panels:
  1. position: commanded (at send time), q_target, q_actual
  2. speed: commanded step / t, qd_target, qd_actual
  3. time between ServoJ sends, and reply time
  4. robot mode
Needs matplotlib (run it on a laptop, not necessarily on the robot computer).
"""
import argparse
import csv
import json
import os


def load_csv(path):
    with open(path) as f:
        return list(csv.DictReader(f))


def fnum(v):
    try:
        return float(v)
    except (TypeError, ValueError):
        return None


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument('run_dir')
    p.add_argument('--joint', type=int, default=None, help='default: the joint in summary.json, else 6')
    p.add_argument('--show', action='store_true')
    args = p.parse_args()

    import matplotlib
    if not args.show:
        matplotlib.use('Agg')
    import matplotlib.pyplot as plt

    joint = args.joint
    summary_path = os.path.join(args.run_dir, 'summary.json')
    servo_t = None
    if os.path.exists(summary_path):
        with open(summary_path) as f:
            summary = json.load(f)
        if isinstance(summary, dict):
            joint = joint or summary.get('joint')
    result_path = os.path.join(args.run_dir, 'result.json')
    if os.path.exists(result_path):
        with open(result_path) as f:
            servo_t = json.load(f).get('config', {}).get('servo_t')
    joint = joint or 6
    j = joint

    cmds = load_csv(os.path.join(args.run_dir, 'commands.csv'))
    rt = load_csv(os.path.join(args.run_dir, 'realtime.csv'))
    if not cmds or not rt:
        raise SystemExit('empty run')
    t0 = float(cmds[0]['t_send'])

    ct = [float(r['t_send']) - t0 for r in cmds]
    cq = [float(r['target_%d' % j]) for r in cmds]
    cv = [0.0] + [(b - a) / servo_t for a, b in zip(cq, cq[1:])] if servo_t else None
    gaps = [(b - a) * 1e3 for a, b in zip(ct, ct[1:])]
    rtt = [fnum(r['rtt_ms']) for r in cmds]
    rej = [(float(r['t_send']) - t0, float(r['target_%d' % j])) for r in cmds if r['err'] not in ('0', '')]

    rt_t = [float(r['host_t']) - t0 for r in rt]
    col = lambda name: [float(r['%s_%d' % (name, j)]) for r in rt]
    mode = [int(float(r['robot_mode'])) for r in rt]

    fig, ax = plt.subplots(4, 1, sharex=True, figsize=(12, 10),
                           gridspec_kw={'height_ratios': [3, 3, 2, 1]})
    ax[0].plot(ct, cq, '.-', ms=3, label='commanded')
    ax[0].plot(rt_t, col('q_target'), label='q_target')
    ax[0].plot(rt_t, col('q_actual'), label='q_actual')
    if rej:
        ax[0].plot([r[0] for r in rej], [r[1] for r in rej], 'rx', ms=10, label='rejected')
    ax[0].set_ylabel('J%d position, deg' % j)
    ax[0].legend(loc='best')

    if cv:
        ax[1].step(ct, cv, where='post', label='commanded step / t')
    ax[1].plot(rt_t, col('qd_target'), label='qd_target')
    ax[1].plot(rt_t, col('qd_actual'), label='qd_actual')
    ax[1].set_ylabel('J%d speed, deg/s' % j)
    ax[1].legend(loc='best')

    ax[2].plot(ct[1:], gaps, '.', ms=3, label='gap between sends')
    ax[2].plot(ct, rtt, '.', ms=3, label='reply time')
    ax[2].set_ylabel('ms')
    ax[2].legend(loc='best')

    ax[3].step(rt_t, mode, where='post')
    ax[3].set_ylabel('mode')
    ax[3].set_xlabel('s since first ServoJ')
    for a in ax:
        a.grid(True, alpha=0.3)
    fig.suptitle(os.path.normpath(args.run_dir))
    fig.tight_layout()
    out = os.path.join(args.run_dir, 'plot_J%d.png' % j)
    fig.savefig(out, dpi=110)
    print(out)
    if args.show:
        plt.show()


if __name__ == '__main__':
    main()

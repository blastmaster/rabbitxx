#! /usr/bin/env python3

import sys
from subprocess import Popen, PIPE
import numpy as np
import matplotlib.pyplot as plt

from pprint import pprint


class KindTimes:

    def __init__(self, kinds=[], times=[], total=0):
        self.kinds = kinds
        self.times = times
        self.total = total


def read_file(filename):

    with open(filename, 'r') as f:
        for line in f.readlines():
            yield line.strip()


def process(cmd, tf_arg):

    with Popen([cmd, tf_arg], stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            yield line.strip()


def read_set(gen):

    kinds = []
    times = []
    for line in gen:
        if not line:
            yield KindTimes(kinds, times, total_time)
            kinds = []
            times = []
            total_time = 0
            continue
        if line.startswith('sum'):
            total_time = int(line.split(' ')[-1][:-2])
            continue
        kind, time = line.split(' ')
        time = int(time[:-2])
        kinds.append(kind)
        times.append(time)


def percentage_times(kt):

    res = []
    for t in kt.times:
        d = kt.total / 100
        p = t / d
        res.append(p)

    return res


def main():

    # if len(sys.argv) < 2:
        # sys.exit("Error not enough arguments. Usage: {} <tracfile>".format(sys.argv[0]));

    cmd = './modules/significant_op_typ/significant_op_type'
    # output = process(cmd, sys.argv[1])
    file = 'madbench_sig_kinds.txt'
    output = read_file(file)

    ops = list(read_set(output))
    for idx, op in enumerate(ops):
        print("Set {}: kinds {} times {} total {}".format(idx+1, op.kinds, op.times, op.total))

    p = percentage_times(ops[-1])
    print(p)
    for idx, op in enumerate(ops):
        fig1, ax1 = plt.subplots()
        ax1.pie(percentage_times(op), labels=op.kinds, autopct='%1.1f%%')
        ax1.axis('equal') # needed that it looks like a circle
        # plt.show()
        name = 'significant_kind-{}.png'.format(idx)
        plt.savefig(name, bbox_inces='tight')

if __name__ == '__main__':
    main()

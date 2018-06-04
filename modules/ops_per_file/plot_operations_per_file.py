#! /usr/bin/env python3

import sys
from subprocess import Popen, PIPE
import numpy as np
import matplotlib.pyplot as plt

from pprint import pprint


class OpsPerFile:

    def __init__(self, filename, operations=[], counts=[]):
        self.filename = filename
        self.operations = operations
        self.counts = counts


def process(cmd, tf_arg):

    with Popen([cmd, tf_arg], stdout=PIPE, bufsize=1, universal_newlines=True) as p:
        for line in p.stdout:
            yield line.strip()

def parse_ops(line):

    ops = []
    cnts = []
    if not line:
        return
    ar = line.split(' ')
    for e in ar:
        op, cnt = e.split(':')
        ops.append(op)
        cnts.append(cnt)
    return (ops, cnts)

def read(gen):
    res = []
    while True:
        try:
            filename = next(gen)
            # print("filename: {}".format(filename))
            ops, cnts = parse_ops(next(gen))
            if filename.startswith('/sys') or filename.startswith('/proc'):
                continue
            res.append(OpsPerFile(filename, ops, cnts))
        except StopIteration:
            return res


def main():

    if len(sys.argv) < 2:
        sys.exit("Error not enough arguments. Usage: {} <tracfile>".format(sys.argv[0]));

    cmd = './modules/ops_per_file/ops_per_file'
    output = process(cmd, sys.argv[1])

    ops = read(output)
    for idx, op in enumerate(ops):
        print("Set {}: filename: {} operations {} cnts {}".format(idx+1, op.filename, op.operations, op.counts))

    for idx, ops in enumerate(ops):
        fig, ax = plt.subplots()
        # pltidx = 220 + (idx  % 4) + 1
        # ax = fig.add_subplot(pltidx)
        # ax.xlabel('I/O Operation')
        # ax.ylabel('Number of Invocations within Set')
        print(ops.operations, ops.counts)
        ax.bar(ops.operations, ops.counts)
        # ax.bar(ops.files, ops.counts)
        name = 'ops_per_file-{}.png'.format(idx)
        plt.savefig(name, bbox_inces='tight')


if __name__ == '__main__':
    main()

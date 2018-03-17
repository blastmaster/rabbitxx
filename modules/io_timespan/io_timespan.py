#! /usr/bin/env python3

import sys
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np

def make_line_generator(filename):

    with open(filename, 'r') as f:
        for line in f.readlines():
            yield str.encode(line)

def read_totals(fgen):

    start = bytes.decode(next(fgen))
    end = bytes.decode(next(fgen))
    ts = int(start.split()[-1][:-2]) / 1000000
    te = int(end.split()[-1][:-2]) / 1000000

    return (ts, te)


def read_file(filename):

    gen = make_line_generator(filename)
    start_time, end_time = read_totals(gen)
    pconv = lambda s: int(s)
    dconv = lambda s: int(s[:-2]) / 1000000
    data = np.genfromtxt(gen, converters={0: pconv, 1: dconv, 2: dconv},
            names=['process', 'start', 'stop', 'region'], dtype=None)
    return (start_time, end_time, data)


def timelines(y, xstart, xstop, color='b'):

    plt.hlines(y, xstart, xstop, color, lw=2)
    plt.vlines(xstart, y+0.03, y-0.03, color, lw=2)
    plt.vlines(xstop, y+0.03, y-0.03, color, lw=2)


if len(sys.argv) < 2:
    print("Error input file needed")
    sys.exit(1)

filename = sys.argv[1]
start_time, end_time, data = read_file(filename)
proc, start, stop = data['process'], data['start'], data['stop']
process, unique_idx = np.unique(proc, True)
y = (proc + 1) / float(len(process) + 1)
yticks = y[unique_idx]
timelines(y, start, stop)
delta = (end_time - start_time) / 10
# xfmt = mdates.DateFormatter("%10.2f")
# xfmt = matplotlib.ticker.FuncFormatter(lambda x, p: format(x, '.2f8'))
ax = plt.gca()
# ax.xaxis.set_major_formatter(xfmt)
plt.yticks(yticks, process)
plt.ylim(0, 1)
plt.ylabel('Processes')
plt.xlim(start_time-delta, end_time+delta)
plt.xlabel('Time in Milliseconds')
plt.show()

#! /usr/bin/env python3

import sys
import matplotlib.patches as mpatches
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
import numpy as np

fig_width = 10.82
fig_height = 6.1
plt.rcParams['figure.figsize'] = fig_width, fig_height

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
    data = np.genfromtxt(gen,
            converters={0: pconv, 1: dconv, 2: dconv},
            names=['process', 'start', 'stop', 'region', 'kind'], dtype=None)
    return (start_time, end_time, data)


def timelines(y, xstart, xstop, color='b'):

    plt.hlines(y, xstart, xstop, color, lw=2)
    # plt.vlines(xstart, y+0.03, y-0.03, color, lw=2)
    # plt.vlines(xstop, y+0.03, y-0.03, color, lw=2)


def plot_timelines(y, kind_data, color):

    start, stop = kind_data['start'], kind_data['stop']
    timelines(y, start, stop, color)


filename = None
if len(sys.argv) < 2:
    filename = "/home/soeste/code/rabbitxx/scripts/ior2-128k-14408347.txt"
    print("using default file: {}".format(filename))
else:
    filename = sys.argv[1]

fig = plt.figure()
start_time, end_time, data = read_file(filename)
proc, start, stop, kind = data['process'], data['start'], data['stop'], data['kind']

read = np.array([t for t in data if bytes.decode(t[4]) == 'read'])
write = np.array([t for t in data if bytes.decode(t[4]) == 'write'])
other = np.array([t for t in data if bytes.decode(t[4]) != 'read' and bytes.decode(t[4]) != 'write'])

process, unique_idx = np.unique(proc, True)
y = (proc + 1) / float(len(process) + 1)
yticks = y[unique_idx]

plot_timelines(y, write, 'y')
plot_timelines(y, read, 'r')
plot_timelines(y, other, 'b')

if start_time < 0:
    start_time = 0
delta = (end_time - start_time) / 10
# xfmt = mdates.DateFormatter("%10.2f")
# xfmt = matplotlib.ticker.FuncFormatter(lambda x, p: format(x, '.2f8'))
ax = plt.gca()
# ax.xaxis.set_major_formatter(xfmt)
# plt.title('I/O Timespan', fontsize=20)
plt.yticks(yticks, process)
plt.ylim(0, 1)
plt.ylabel('Processes', fontsize=20)
print("start_time: {}".format(start_time))
print("end_time: {}".format(end_time))
plt.xlim(start_time-delta, end_time+delta)
plt.xlabel('Time (ms)', fontsize=20)
ax.tick_params(axis='x', labelsize=12)
ax.tick_params(axis='y', labelsize=12)
write_patch = mpatches.Patch(color='y', label='write-op')
read_patch = mpatches.Patch(color='r', label='read-op')
other_patch = mpatches.Patch(color='b', label='metadata-op')
plt.legend(handles=[write_patch, read_patch, other_patch], loc=2)
# plt.legend(handles=[write_patch, read_patch], loc=2)
# plt.grid(True)
plt.show()
# fig.savefig('io_timespan_mcolor.png', bbox_inches='tight')

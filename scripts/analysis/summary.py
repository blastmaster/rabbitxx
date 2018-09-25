#! /usr/bin/env python3

from analysis.experiment import Experiment, ExperimentStats, read_experiment

import matplotlib.pyplot as plt
import numpy as np


def build_time_summary(exp: Experiment):
    '''
        Return a list containing the build times of each step of the CIO-Set algorithm.
        This is usually the build times for:
            * graph construction (contains the time for read the trace file)
            * build local CIO-Sets (contains the depth-first-visit)
            * merge to global CIO-Sets
    '''
    graph_bt = exp.experiment_stats.graph_stats.build_time
    local_cio_set_bt = exp.experiment_stats.pio_stats.build_time
    cio_set_bt = exp.experiment_stats.cio_stats.build_time

    return [graph_bt, local_cio_set_bt, cio_set_bt]


def build_time_table(exp_stats: ExperimentStats):

    print("Graph construction: {}".format(exp_stats.graph_stats.build_time))
    print("local CIO-Sets: {}".format(exp_stats.pio_stats.build_time))
    print("global CIO-Sets: {}".format(exp_stats.cio_stats.build_time))


def plot_build_times(bt_l, filename: str):

    N = len(bt_l)
    ind = np.arange(N)
    width = 0.35
    fig, ax = plt.subplots()
    b_plt = ax.bar(ind, bt_l, width)

    def autolabel(rects):
        for i, rect in enumerate(rects):
            height = rect.get_height()
            ax.text(rect.get_x() + (rect.get_width() / 10.), 1.01 * height, '{}'.format(bt_l[i]))

    autolabel(b_plt)
    plt.xticks(ind, ('graph construction', 'local CIO-Sets', 'CIO-Sets (merged)'))
    plt.ylabel('Time')
    plt.xlabel('Step')
    # plt.legend(('graph construction', 'local CIO-Sets', 'CIO-Sets (merged)'))
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')


if __name__ == '__main__':
    import sys
    filename = 'build_time.png'
    if len(sys.argv) < 2:
        sys.exit('Usage: {} <path-to-experiment> [filename]'.format(sys.argv[0]))
    if sys.argv[2]:
        filename = sys.argv[2]
    exp = read_experiment(sys.argv[1])
    build_time_list = build_time_summary(exp)
    build_time_table(exp.experiment_stats)
    plot_build_times(build_time_list, filename)

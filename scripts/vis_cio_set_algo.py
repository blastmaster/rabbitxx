#! /usr/bin/env python3

from analysis.experiment import ExperimentStats, read_experiment_stats

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import argparse

from typing import List

''' viz_cio_set_algo.py
Script to compare phase of the CIO-Set algorithm.
Output is a simple stacked bar plot showing the duration of the CIO-Set algorithm.

Example:
$ ./vis_cio_set_algo.py ./madbench2-shared-16 ./madbench2-shared-64 --xlabels madbench-16 madbench-64 -o madbench-out.png
'''

class AlgorithmStats:
    graph_construction_times_: List[float]
    phase_construction_times_: List[float]
    merge_times_: List[float]
    len_: int

    def __init__(self, exp_stats: List[ExperimentStats]) -> None:
        self.len_ = len(exp_stats)
        self.graph_construction_times_ = [st.graph_stats.build_time / 1000. for st in exp_stats]
        self.phase_construction_times_ = [st.pio_stats.build_time / 1000. for st in exp_stats]
        self.merge_times_ = [st.cio_stats.build_time / 1000. for st in exp_stats]

    @property
    def graph_times(self) -> List[float]:
        return self.graph_construction_times_

    @property
    def phase_times(self) -> List[float]:
        return self.phase_construction_times_

    @property
    def merge_times(self) -> List[float]:
        return self.merge_times_

    @property
    def len(self) -> int:
        return self.len_


def plot_algorithm_stats(algo_stats: AlgorithmStats, xticks: List[str], filename: str):

    # Size adjustments, for better pictures in the paper.
    # 3.5 inch per column
    # 7.15 inches per textwidth
    size_mult = 4
    # width in inches
    # fig_width = 4.1
    fig_width = 3.5 * size_mult
    # height in inches
    # fig_height = 1.8
    fig_height = 1.5 * size_mult
    # font size
    fig_font_size = 6 * size_mult

    # set canvas size
    plt.rcParams['figure.figsize'] = fig_width, fig_height

    # set font size and style
    matplotlib.rcParams.update({'font.size': fig_font_size})
    matplotlib.rcParams.update({'axes.linewidth': 0.5})

    ind = np.arange(algo_stats.len)
    width = 0.35
    fig, ax = plt.subplots()

    gc = ax.bar(ind, algo_stats.graph_times, width, color='g')
    spp = ax.bar(ind, algo_stats.phase_times, width, bottom=algo_stats.graph_times, color='b')
    mg = ax.bar(ind, algo_stats.merge_times, width,
            bottom=np.array(algo_stats.graph_times) + np.array(algo_stats.phase_times),
            color='r')

    totals = [g + p + m for g, p, m in zip(algo_stats.graph_times, algo_stats.phase_times, algo_stats.merge_times)]
    def autolabel(rects, div=2):
        for i, rect in enumerate(rects):
            total = totals[i]
            height = rect.get_height()
            ax.text(rect.get_x() + (rect.get_width() / 4.), rect.get_y() * 1.08,
                    '{} ms'.format(int(total)))

    autolabel(mg)
    plt.xticks(ind, xticks)
    ax.set_ylim(0, 230000)
    plt.ylabel('Time in ms')
    plt.xlabel('Trace configuration')
    plt.legend(('Graph construction', 'per-process CIO-Sets', 'Merge'))
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')


def main(args) -> None:

    exp_stats = [read_experiment_stats(path) for path in args.experiment_paths]
    algo_stats = AlgorithmStats(exp_stats)
    xticks = []
    if not args.xlabels:
        xticks = [st.tracefile for st in exp_stats]
    else:
        xticks = args.xlabels

    plot_algorithm_stats(algo_stats, xticks, args.output)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'experiment_paths',
            nargs='+',
            type=str,
            help='Paths of exepriments to compare.',
    )
    parser.add_argument(
            '--xlabels',
            nargs='*',
            type=str,
            help='List of xticks.',
    )
    parser.add_argument(
            '-o', '--output',
            type=str,
            default='cio_set_algo.png',
            help='Filename for the resulting png.',
    )

    args = parser.parse_args()
    main(args)

#! /usr/bin/env python3

import os
import sys
import argparse

from analysis import experiment
from analysis.accessTracking import prefix_file_filter, OffsetTracker, track_file_offsets, recalculate_offset_of_set, plot_access_range, NonExistentFileError, get_pids
from analysis.utils import byte_to_kilobyte_label, gen_set_labels

import matplotlib.pyplot as plt

TEST_FILE_NAME=' /home/h0/soeste/MADbench2/files/data'

def do_list_files(file_map):
    [print(f) for f in file_map.keys()]


def plot_rws_range(group) -> None:

    #FIXME to decide which kind should be printed pass this list of tuples as argument
    # for op_kind, color in [(' read', 'b'), (' write', 'r'), (' seek', 'y')]:
    for op_kind, color in [(' read', 'b'), (' write', 'r')]:
        krows = group[group['kind'] == op_kind]
        if krows.empty:
            print('DEBUG {} kind is empty'.format(op_kind))
            continue
        print('DEBUG processing kind {}'.format(op_kind))
        for _, row in krows.iterrows():
            plot_access_range(row['pid'], row, color=color)

def plot_cio_set_access_pattern(cio_set, set_label: str) -> None:
    ''' Plot access pattern for a given file.
        X-Axis: The accessed range in the file.
        Y-Axis: The process accessing the file.
        The different operation kinds are color-coded.

    '''
    try:
        print('DEBUG processing: {}'.format(set_label))
        # print('DBEUG set: {}'.format(cio_set))
        fig, ax = plt.subplots()
        # TODO pass filename as argument
        for group in recalculate_offset_of_set(cio_set, TEST_FILE_NAME):
            plot_rws_range(group)

        min_response_size = group['response_size'].min()
        max_response_size = group['response_size'].max()
        mean_response_size = group['response_size'].mean()
        print('DEBUG mean_response_size {}'.format(int(mean_response_size)))
        print('DEBUG min_response_size {}'.format(int(min_response_size)))
        print('DEBUG max_response_size {}'.format(int(max_response_size)))

        min_offset = group['offset'].min()
        max_offset = group['offset'].max()

        print('DEBUG max_offset {}'.format(max_offset))

        xmin = min_offset - mean_response_size if (min_offset - mean_response_size) > 0 else 0
        xmax = max_offset + mean_response_size

        # TODO set useful xticks
        print('DEBUG xmin: {} xmax: {}'.format(xmin, xmax))
        # plt.xlim(xmin, xmax)
        print('DEBUG ytick pids: {}'.format(get_pids(cio_set)))
        plt.yticks(get_pids(cio_set))

        fig.canvas.draw()

        # convert xticklabel text to kilobyte
        # xlabels = [byte_to_kilobyte_label(item.get_text()) for item in ax.get_xticklabels()]
        # ax.set_xticklabels(xlabels)
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)

        plt.ylabel('Process')
        plt.xlabel('File range')
        plt.savefig('./{}-access_pattern.png'.format(set_label))
    except NonExistentFileError as e:
        print('INFO: file {} could not be found in {} ... going to next set'
                .format(TEST_FILE_NAME, set_label))
        return


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="Base path to rabbitxx-experiment results directory.", type=str)

    args = parser.parse_args()
    if not os.path.exists(args.path):
        sys.exit("Given base path does not exist.")

    run = experiment.read_experiment(args.path)

    # for cio_set, set_label in zip(run.cio_sets, gen_set_labels(run.cio_sets)):
        # plot_cio_set_access_pattern(cio_set, set_label)
    plot_cio_set_access_pattern(run.cio_sets[3], 'set4test-without-seek')

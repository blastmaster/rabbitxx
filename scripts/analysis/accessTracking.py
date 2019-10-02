import pandas as pd
import numpy as np

from typing import Tuple

from .utils import *


def prefix_file_filter(file_map, prefix: str):
    return {file: fs for file, fs in file_map.items() if file.startswith(prefix)}


def calculate_access_range(row) -> Tuple:
    ''' return a tuple (start_offset, end_offset) of the operation given in row '''
    if row['offset'] < 0.0:
        raise ValueError("offset less then zero!")
    start_offset = row['offset'] - row['response_size']
    if start_offset < 0.0:
        return 0.0, row['offset']
    return start_offset, row['offset']


class NonExistentFileError(Exception):
    pass


def get_file_acccess_per_process(setdf, filename: str, pids=None):
    '''
        Returns a generator for all accesses of the given processes for the
        given file.
    '''

    pids = get_processes(setdf, filename) if pids is None else pids
    for pid in pids:
        group = setdf.groupby(['pid', 'filename']).get_group((pid, filename))
        group = add_hash_column(group)
        yield group


''' ==================== Plotting ==================== '''


def plot_range(y, row, **kwargs) -> None:
    ''' yaxis: response_size
        xaxis: file range '''
    start_of, end_of = calculate_access_range(row)
    print("DEBUG (start, end) -> ({}, {})".format(start_of, end_of))
    timelines(y, start_of, end_of, **kwargs)


# TODO do it with `pid` as y-axis
def plot_rws_range(group) -> None:

    #FIXME to decide which kind should be printed pass this list of tuples as argument
    # for op_kind, color in [(' read', 'b'), (' write', 'r'), (' seek', 'y')]:
    for op_kind, color in [(' read', 'b'), (' write', 'r'), (' seek', 'k')]:
        krows = group[group['kind'] == op_kind]
        if krows.empty:
            print('DEBUG {} kind is empty'.format(op_kind))
            continue
        print('DEBUG processing kind {}'.format(op_kind))
        for _, row in krows.iterrows():
            plot_range(row['pid'], row, color=color)


''' The `plot_rws_range` function tries to show read, write and seek accesses if they are available.
    It looks like that the different operations shadow the accesses of other kind of operations.
    E.g. seek operations may shadow write operations.
'''

def plot_access(group, kind=' read', color='b') -> None:

    if not kind in group['kind'].values:
        raise ValueError("ERROR no kind: {} available!".format(kind))

    # filter operations by kind
    ops = group[group['kind'] == kind]
    for _, row in ops.iterrows():
        # plot range for each access
        plot_range(row['pid'], row, color=color)


#FIXME
def plot_response_size_range(group) -> None:

    for op_kind, color in [(' read', 'b'), (' write', 'r'), (' seek', 'y')]:
        krows = group[group['kind'] == op_kind]
        if krows.empty:
            print('DEBUG {} kind is empty'.format(op_kind))
            continue
        print('DEBUG processing kind {}'.format(op_kind))
        for _, row in krows.iterrows():
            plot_range(row['response_size'], row, color=color)


def plot_cio_set_access_pattern(cio_set, set_label: str, file_name: str, processes=None) -> None:
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
        for group in get_file_acccess_per_process(cio_set, file_name, processes):
            plot_rws_range(group) # FIXME plots all accesses

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
            plt.xlim(xmin, xmax)
            print('DEBUG ytick pids: {}'.format(get_processes(cio_set, file_name)))
            plt.yticks(get_processes(cio_set, file_name))

            fig.canvas.draw()

        # convert xticklabel text to kilobyte
        # xlabels = [byte_to_kilobyte_label(item.get_text()) for item in ax.get_xticklabels()]
        xlabels = [item.get_text() for item in ax.get_xticklabels()]
        ax.set_xticklabels(xlabels)
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)

        plt.ylabel('Process')
        plt.xlabel('File range')
        plt.savefig('./{}-access_pattern.png'.format(set_label))
    except NonExistentFileError as e:
        print('INFO: file {} could not be found in {} ... going to next set'
                .format(file_name, set_label))
        return


def plot_cio_set_response_size_pattern(cio_set, set_label: str, file_name :str, pids=None) -> None:

    fig, ax = plt.subplots()

    for group in get_file_acccess_per_process(cio_set, file_name, pids):
        #TODO do plotting!
        plot_response_size_range(group)
        # kgroup = group.pipe(Filter.read_kinds)

        min_response_size = group['response_size'].min()
        max_response_size = group['response_size'].max()
        mean_response_size = group['response_size'].mean()
        print('DEBUG mean_response_size {}'.format(int(mean_response_size)))
        print('DEBUG min_response_size {}'.format(int(min_response_size)))
        print('DEBUG max_response_size {}'.format(int(max_response_size)))

        min_offset = group['offset'].min()
        max_offset = group['offset'].max()

        plt.ylabel('Response size')
        plt.xlabel('File range')

        # xlim
        xmin = min_offset - mean_response_size if (min_offset - mean_response_size) > 0 else 0
        xmax = max_offset + mean_response_size
        plt.xlim(xmin, xmax)
        print('xmin: {} xmax: {}'.format(xmin, xmax))
        # ylim
        ymin = min_response_size - mean_response_size if (min_response_size - mean_response_size) > 0 else 0
        ymax = max_response_size + mean_response_size

        plt.ylim(ymin, ymax)
        print('ymin: {} ymax: {}'.format(ymin, ymax))

        fig.canvas.draw()

        # convert xticklabel text to kilobyte
        # xlabels = [byte_to_kilobyte_label(item.get_text()) for item in ax.get_xticklabels()]
        # ax.set_xticklabels(xlabels)
        for tick in ax.get_xticklabels():
            tick.set_rotation(45)

        # convert ytickslabel text to kilobyte
        # ylabels = [byte_to_kilobyte_label(item.get_text()) for item in ax.get_yticklabels()]
        # ax.set_yticklabels(ylabels)

        plt.savefig('./{}-response_size_access_pattern.png'.format(set_label))

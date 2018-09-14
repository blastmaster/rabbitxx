import pandas as pd
import numpy as np

from .utils import *

class OffsetTracker:
    value: int = 0

    def __init__(self, initial: int=0) -> None:
        self.value = initial

    def __repr__(self) -> str:
        return 'OffsetTracker value={}'.format(self.value)

    @property
    def offset(self) -> int:
        return self.value

    @offset.setter
    def offset(self, value: int) -> None:
        self.value = value


def track_file_offsets(row, **kwds):

    tracker = kwds['tracker']

    if row['kind'] == ' create':
        # print('create')
        assert tracker.offset == 0, "Initial offset unequal to zero on create operation"

    elif row['kind'] == ' seek':
        # print('seek')
        # TODO consider seek options {seek_set, seek_cur, seek_end}
        if row['option'] == ' start': # seek_set
            tracker.offset = row['response_size']
        if row['option'] == 'current': # seek_cur
            tracker.offset += row['response_size']
        if row['option'] == 'end': # seek_end
            raise Exception("Unhandled seek option {} cannot track offset!".format(row['option']))
        if row['option'] == 'data': # seek_data
            raise Exception("Unhandled seek option {} cannot track offset!".format(row['option']))
        if row['option'] == 'hole': # seek_hole
            raise Exception("Unhandled seek option {} cannot track offset!".format(row['option']))

    elif row['kind'] == ' write' or row['kind'] == ' read':
        # print('read/write')
        tracker.offset += row['response_size']

    elif row['kind'] == ' close_or_delete':
        # return current offset but reset tracker!
        tmp = tracker.offset
        tracker.offset = 0
        return tmp

    return int(tracker.offset)


def prefix_file_filter(file_map, prefix: str):
    return {file: fs for file, fs in file_map.items() if file.startswith(prefix)}


def calculate_access_range(row):
    ''' return a tuple (start_offset, end_offset) of the operation given in row '''
    start_offset = row['offset'] - row['response_size']
    if start_offset < 0.0:
        return 0.0, row['offset']
    return start_offset, row['offset']


def plot_access_range(y, row, **kwargs) -> None:
    ''' yaxis: response_size
        xaxis: file range '''
    start_of, end_of = calculate_access_range(row)
    timelines(y, start_of, end_of, **kwargs)


class NonExistentFileError(Exception):
    pass


def recalculate_offset_of_set(setdf, filename: str, pids=None):
    ''' Calculate the offsets for all pids in set of given filename.
        Returns an generator over all processes
    '''
    pids = get_processes(setdf, filename) if pids is None else pids
    print('DEBUG pids: {}'.format(pids))
    for pid in pids:
        print('DEBUG pid {} filename {}'.format(pid, filename))
        # Check if the filename we looking for is existent in the given set
        if not filename in setdf['filename'].values:
            # TODO fix exeception text, in what set?!?
            raise NonExistentFileError('{} does not exists in set'.format(filename))

        try:
            file_group = setdf.groupby(['pid', 'filename']).get_group((pid, filename))
            file_group['offset'] = file_group.apply(track_file_offsets, axis=1, tracker=OffsetTracker())
            yield file_group
        except KeyError:
            continue   # or  yield [] ?


''' ==================== Plotting ==================== '''

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
            plot_access_range(row['pid'], row, color=color)


#FIXME
def plot_response_size_range(group) -> None:

    for op_kind, color in [(' read', 'b'), (' write', 'r'), (' seek', 'y')]:
        krows = group[group['kind'] == op_kind]
        if krows.empty:
            print('DEBUG {} kind is empty'.format(op_kind))
            continue
        print('DEBUG processing kind {}'.format(op_kind))
        for _, row in krows.iterrows():
            plot_access_range(row['response_size'], row, color=color)


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
        for group in recalculate_offset_of_set(cio_set, file_name, processes):
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
        plt.xlim(xmin, xmax)
        print('DEBUG ytick pids: {}'.format(get_processes(cio_set, file_name)))
        plt.yticks(get_processes(cio_set, file_name))

        fig.canvas.draw()

        # convert xticklabel text to kilobyte
        xlabels = [byte_to_kilobyte_label(item.get_text()) for item in ax.get_xticklabels()]
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

    for group in recalculate_offset_of_set(cio_set, file_name, pids):
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

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
        tracker.offset += row['response_size']

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

# def do_offset_tracking(exp, prefix):

    # fl = []
    # filtered_files = prefix_file_filter(exp.experiment_stats.file_map, prefix)
    # for f in filtered_files.keys():
        # f = ' {}'.format(f) # tweak whitespaces!
        # #FIXME
        # f_group = s.groupby('filename').get_group(f)
        # f_group['offset'] = f_group.apply(track_file_offsets, axis=1, tracker=OffsetTracker())
        # fl.append(f_group) 

    # return fl

class NonExistentFileError(Exception):
    pass


def get_pids(setdf):
    ''' Return sorted array containing all pids involved in the set '''
    return np.array(sorted(setdf['pid'].unique()))


def recalculate_offset_of_set(setdf, filename: str, pids=None):
    ''' Calculate the offsets for all pids in set of given filename. '''

    pids = get_pids(setdf) if pids is None else pids
    print('DEBUG pids: {}'.format(pids))
    for pid in pids:
        print('DEBUG pid {} filename {}'.format(pid, filename))
        # Check if the filename we looking for is existent in the given set
        if not filename in setdf['filename'].values:
            # TODO fix exeception text, in what set?!?
            raise NonExistentFileError('{} does not exists in set'.format(filename))

        file_group = setdf.groupby(['pid', 'filename']).get_group((pid, filename))
        file_group['offset'] = file_group.apply(track_file_offsets, axis=1, tracker=OffsetTracker())
        yield file_group


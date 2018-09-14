import pandas as pd
import matplotlib.pyplot as plt

from typing import List, Dict

''' CIO-Set Helpers '''
def get_processes(cio_set: pd.DataFrame, filename: str) -> List[int]:
    ''' Return a list of pids accessing the file given by filename. '''
    try:
        grouped = cio_set.groupby('filename')
        res = grouped.get_group(filename)['pid'].unique()
        res.sort()
        return res.tolist()
    except KeyError:    # if a set is given which does not touch the file.
        return []


def get_files_in_set(cio_set: pd.DataFrame) -> List[str]:
    ''' Return unique filenames accessed in `cio_set`. '''
    return cio_set['filename'].unique().flatten().tolist()


''' ====================General Helpers ==================== '''

def microseconds_to_seconds(microseconds) -> float:
    ''' Helper converting a microsecond timestamp into seconds. '''
    return microseconds / 1000000.

''' Data '''

def calculate_bandwidth(row):
    ''' Return the calculated bandwidth of the given row in bytes/microseconds. '''
    return row['response_size'] / row['duration']

''' ==================== Plotting Helpers ==================== '''

def timelines(y, xstart, xstop, color='b') -> None:
    ''' Plot a horizontal line from xstart to xstop at yaxis postion y. '''
    # print('y: {} xstart: {} xstop: {}'.format(y, xstart, xstop))
    plt.hlines(y, xstart, xstop, color, lw=12)


def gen_set_labels(set_list):
    ''' Generate the labels for diagrams ['Set 1' 'Set 2' ...] '''
    return ["Set {}".format(str(i)) for i, _ in enumerate(set_list)]

def gen_set_labels_from_idx(idx_list):
    ''' Generate the labels for diagrams ['Set 3' 'Set 4' ...] from a given list of indices. '''
    return ['Set {}'.format(i) for i in idx_list]


def byte_to_kilobyte_label(byte_label: str) -> str:
    ''' Helper function to transform labels of bytes into labels of KB '''
    byte_label_value = int(float(byte_label))
    return '{} KB'.format(int(byte_label_value / 1000))

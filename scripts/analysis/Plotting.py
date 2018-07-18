'''
    This module contains the plotting functions for the anaysis.

'''

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

from .utils import gen_set_labels

''' DOMINANT OPERATION KIND '''

def plot_dominant_kinds(cio_sets, out_name: str) -> None:
    ''' Generate a bar plot for the most dominant operation kind of the CIO-Sets. '''

    N = len(cio_sets)
    ind = np.arange(N)
    width = 0.15
    labels = gen_set_labels(cio_sets)
    filename = '{}-dominant-kind.png'.format(out_name)
    fig, ax = plt.subplots()

    kind_dict = {
            'Metadata': [],
            'Read': [],
            'Write': [],
            'Flush': [],
            'Seek': []
    }

    for sdf in cio_sets:
        meta_df = sdf[(sdf['kind'] == ' create') | (sdf['kind'] == ' close_or_delete')]
        kind_dict['Metadata'].append(meta_df['duration'].aggregate(sum))
        read_df = sdf[sdf['kind'] == ' read']
        kind_dict['Read'].append(read_df['duration'].aggregate(sum))
        write_df = sdf[sdf['kind'] == ' write']
        kind_dict['Write'].append(write_df['duration'].aggregate(sum))
        flush_df = sdf[sdf['kind'] == ' flush']
        kind_dict['Flush'].append(flush_df['duration'].aggregate(sum))
        seek_df = sdf[sdf['kind'] == ' seek']
        kind_dict['Seek'].append(seek_df['duration'].aggregate(sum))

    m_rects = ax.bar(ind, kind_dict['Metadata'], width)
    r_rects = ax.bar(ind+width, kind_dict['Read'], width)
    w_rects = ax.bar(ind+(width*2), kind_dict['Write'], width)
    f_rects = ax.bar(ind+(width*3), kind_dict['Flush'], width)
    s_rects = ax.bar(ind+(width*4), kind_dict['Seek'], width)
    ax.legend(
            (m_rects[0], r_rects[0], w_rects[0], f_rects[0], s_rects[0]),
            ('Metadata', 'Read', 'Write', 'Flush', 'Seek'),
            loc='upper center', bbox_to_anchor=(1.1, 1))

    ax.set_yscale('log')
    ax.set_xticklabels(labels)
    ax.set_xticks(ind + (width*4) / 2)
    plt.xlabel('CIO-Sets')
    plt.ylabel(r'Sum duration in $\mu$s')
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')


''' SET DURATION '''

# TODO need table output
def plot_set_durations(cio_sets, set_durations, out_name: str) -> None:
    ''' Generate a bar plot comparing the duration of the given sets. '''

    labels = gen_set_labels(cio_sets)
    tf = pd.DataFrame({'duration': set_durations})
    tf.set_index([labels])
    fig, ax = plt.subplots()
    ind = np.arange(len(set_durations))
    filename = '{}-set-duration.png'.format(out_name)

    def autolabel(rects) -> None:
        for rect in rects:
            height = rect.get_height()
            ax.text(
                    rect.get_x() + (rect.get_width() / 16.),
                    1.1 * height,
                    r'{}$\mu$s'.format(int(height)))

    rects = ax.bar(ind, tf['duration'])
    ax.set_yscale('log')
    x_range = range(len(cio_sets) + 1)
    print('DEBUG max xtick {}'.format(max(x_range)))
    ax.set_xticks(np.arange(min(x_range), max(x_range), 1.0))
    ax.set_xticklabels(labels)
    autolabel(rects)
    ax.set_ylim(bottom=100, top=1000000)
    plt.xlabel('CIO-Sets')
    plt.ylabel(r'Duration in $\mu$s')
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')


''' NUMBER OF I/O EVENTS '''

# TODO need table output
def plot_num_io_events(cio_sets, out_name: str) -> None:
    ''' Generate a bar plot comparing the number of I/O event in the given sets. '''

    number_of_events = [len(cio_set) for cio_set in cio_sets]
    labels = gen_set_labels(cio_sets)
    filename = '{}-number-of-io-events.png'.format(out_name)
    ind = np.arange(len(number_of_events))
    fig, ax = plt.subplots()

    def autolabel(rects) -> None:
        for rect in rects:
            height = rect.get_height()
            ax.text(
                    rect.get_x() + (rect.get_width() / 3.),
                    1.1*height,
                    '{}'.format(int(height)))

    rects = ax.bar(ind, number_of_events)
    autolabel(rects)
    ax.set_yscale('log')
    x_range = range(len(cio_sets) + 1)
    print('DEBUG max xtick {}'.format(max(x_range)))
    ax.set_xticks(np.arange(min(x_range), max(x_range), 1.0))
    ax.set_xticklabels(labels)
    ax.set_ylim(bottom=1, top=200)
    plt.xlabel('CIO-Set')
    plt.ylabel('Number of I/O Events')
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')



''' REQUEST SIZES '''

def plot_read_request_sizes() -> None:

    raise NotImplementedError


def plot_write_request_sizes() -> None:

    raise NotImplementedError


''' POSIX '''

def plot_io_paradigm(cio_sets, out_name: str) -> None:
    ''' Generate a bar plot for the number of operation per I/O - Paradgim of each set. '''

    #TODO missing MPIIO
    N = len(cio_sets)
    ind = np.arange(N)
    width = 0.25
    labels = gen_set_labels(cio_sets)
    filename = '{}-io-paradigm.png'.format(out_name)
    fig, ax = plt.subplots()
    paradigm_dict = {
            'POSIX': [],
            'ISOC': []}

    for cio_set in cio_sets:
        set_par_dict = cio_set['paradigm'].value_counts().to_dict()
        paradigm_dict['POSIX'].append(set_par_dict.get(' POSIX I/O', 0))
        paradigm_dict['ISOC'].append(set_par_dict.get(' ISO C I/O', 0))

    posix_rects = ax.bar(ind, paradigm_dict['POSIX'], width)
    isoc_rects = ax.bar(ind+width, paradigm_dict['ISOC'], width)
    ax.legend(
            (posix_rects[0], isoc_rects[0]),
            ('POSIX', 'ISO-C'))
    ax.set_xticks(ind + width / 2)
    ax.set_xticklabels(labels)
    plt.xlabel('CIO-Sets')
    plt.ylabel('Number of operations')
    plt.savefig(filename, dpi=300, format='png', bbox_inches='tight')


''' FILE ACCESSES '''

def plot_accessed_files(cio_sets, out_name: str) -> None:
    ''' Generate a bar plot for each file ?!? '''
    raise NotImplementedError


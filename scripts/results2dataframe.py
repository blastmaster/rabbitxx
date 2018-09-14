#! /usr/bin/env python3

import os
import sys
import argparse

from analysis import experiment
from analysis import Filter
from analysis.accessTracking import *
from analysis.utils import byte_to_kilobyte_label, gen_set_labels

import matplotlib.pyplot as plt

TEST_FILE_NAME=' /home/h0/soeste/MADbench2/files/data'

def do_list_files(file_map):
    [print(f) for f in file_map.keys()]


def apply_file_filter(exp):
    ''' Apply a set of filter methods on the sets and override the experiment results by that.
        Returns a generator which produces a tuple of (set_idx, cio_set).
    '''
    #FIXME might be better if we can pass a list of filters to a function an this function applies them all.
    exp.cio_sets = [
        cio_set.pipe(Filter.virtual_fs_filter)
            .pipe(Filter.cgroup_fs_filter)
            .pipe(Filter.sw_filter)
            .pipe(Filter.stdout_filter)
            .pipe(Filter.dev_fs_filter)
            .pipe(Filter.etc_filter)
            .pipe(Filter.filename_filter, fname=' /') for cio_set in exp.cio_sets]
    for idx, cio_set in enumerate(exp.cio_sets):
        if cio_set.empty:
            print("Filter empty set nr: {}".format(idx + 1))
        else:
            yield idx, cio_set


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="Base path to rabbitxx-experiment results directory.", type=str)

    args = parser.parse_args()
    if not os.path.exists(args.path):
        sys.exit("Given base path does not exist.")

    run = experiment.read_experiment(args.path)
    filtered_cio_sets = list(apply_file_filter(run))

    ### ACCESS PATTERN
    #TODO labels might be wrong after filtering!
    for cio_set, set_label in zip(filtered_cio_sets, gen_set_labels(filtered_cio_sets)):
        plot_cio_set_access_pattern(cio_set, set_label, TEST_FILE_NAME)

    for cio_set, set_label in zip(filtered_cio_sets, gen_set_labels(filtered_cio_sets)):
        plot_cio_set_response_size_pattern(cio_set, set_label, TEST_FILE_NAME)


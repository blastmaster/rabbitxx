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


    '''


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("path", help="Base path to rabbitxx-experiment results directory.", type=str)

    args = parser.parse_args()
    if not os.path.exists(args.path):
        sys.exit("Given base path does not exist.")

    run = experiment.read_experiment(args.path)

    # for cio_set, set_label in zip(run.cio_sets, gen_set_labels(run.cio_sets)):
        # plot_cio_set_access_pattern(cio_set, set_label)

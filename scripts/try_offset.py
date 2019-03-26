#! /usr/bin/env python3

import argparse

from analysis import experiment
from analysis import Filter
from analysis.metadata import make_creates
from analysis.overlap import overlaps, get_access_mappings, read_modify_write

import numpy as np


def read_accesses(setidx: int, filename: str, acc_map):
    return [pfa.read_intervals for pfa in acc_map[setidx][filename] if pfa.has_reads()]


def write_accesses(setidx: int, filename: str, acc_map):
    return [pfa.write_intervals for pfa in acc_map[setidx][filename] if pfa.has_writes()]


def report_overlap(ovlp) -> None:
    ''' Print simple report for an found overlap. '''

    print("Overlapping access found in CIO-Set Nr {}".format(ovlp.set_idx))
    print("{} {}".format(ovlp.first.iv, ovlp.second.iv))
    print("Filename: {}".format(ovlp.filename))
    print("Between process {0} and {1}".format(ovlp.first.process, ovlp.second.process))
    print("\n")


def report_concurrent_creates(create_lst, experiment) -> None:
    ''' Print a simple report for concurrent creates in the same subdirectory. '''

    def get_data(exp, sidx, ridx):
        return exp.cio_sets[sidx].loc[ridx]

    for create in create_lst:
        for directory, count in create.creates.items():
            regions = []
            for ridx in create.row_indices:
                data = get_data(experiment, create.set_index, ridx)
                regions.append(data['region_name'])
            ops = np.unique(np.array(regions))
            print("In CIO-Set {}, {} creates in directory: {} operations {}".format(
                create.set_index, count, directory, ops))


def report_distributed_read_modify_write(rmw) -> None:
    ''' Print a simple report for distributed read-modify-write access on the same region of a file. '''

    print("Read from process {} on file {} @ {} -> write from process {} on file {} @ {}".format(
        rmw.first.process, rmw.first.filename, rmw.first.iv,
        rmw.second.process, rmw.second.filename, rmw.second.iv))


def report_experiment_info(experiment) -> None:
    ''' Print an overview report for the rabbitxx-experiment. '''

    print("Experiment:")
    print("Applicaton run duration: {:.2f}s".format(experiment.experiment_length()))
    print("Number of Locations: {}".format(experiment.num_locations()))
    print("Trace file: {}".format(experiment.tracefile()))
    print("CIO-Sets:")
    print("Number of CIO-Sets: {}".format(experiment.num_cio_sets()))
    print("File Map:")
    for file, fs in experiment.file_map().items():
        print("\t{} : {}".format(file, fs))


def dump_set_files(set_files) -> None:
    ''' Dump file accessess of every set. '''

    for setidx, file_acc in set_files.items():
        print("Accessess in set {}".format(setidx))
        for filename, acc_list in file_acc.items():
            print("Accesses to {}".format(filename))
            for acc in acc_list:
                print(acc)


def main(args) -> None:
    exp = experiment.read_experiment(args.experiment_dir)
    exp.filter(Filter.file_filter)
    # dump_set_files(set_files)

    if args.info:
        report_experiment_info(exp)

    if args.concurrent_creates:
        print("Analyze concurrent creates ...")
        creates = [ make_creates(sidx, cio_set) for sidx, cio_set in enumerate(exp.cio_sets) ]
        report_concurrent_creates(creates, exp)

    if args.overlap or args.read_modify_write:
        set_files = get_access_mappings(exp)

        if args.overlap:
            print("Analyze overlapping accesses ...")
            for acc_m in set_files:
                print("checking set idx: {}".format(acc_m.set_index))
                for file, acc_l in acc_m.file_accesses.items():
                    for ovlp in overlaps(file, acc_l):
                        report_overlap(ovlp)
                    else:
                        print("No overlapping access!")

        if args.read_modify_write:
            print("Analyze distributed read-modify-write ...")
            for acc_m in set_files:
                print("checking set idx: {}".format(acc_m.set_index))
                for file, acc_l in acc_m.file_accesses.items():
                    if acc_m.num_processes(file) < 2:
                        continue
                    for rmw in read_modify_write(acc_l):
                        report_distributed_read_modify_write(rmw)
                    else:
                        print("No distributed read-modify-write found.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
            'experiment_dir',
            type=str,
            help="Path to the rabbitxx-experiment directory.",
    )
    parser.add_argument(
            '-i', '--info',
            action='store_true',
            default=False,
            help='Report some basic information about the experiment.',
    )
    parser.add_argument(
            '-c', '--concurrent-creates',
            action='store_true',
            default=False,
            help='Report concurrent creates in the same subdirectory.',
    )
    parser.add_argument(
            '-ov', '--overlap',
            action='store_true',
            default=False,
            help='Report overlapping accesses read or write on the same region of a file.',
    )
    parser.add_argument(
            '-rmw', '--read-modify-write',
            action='store_true',
            default=False,
            help='Report distributed read-modify-write access to the same region of a file.',
    )

    args = parser.parse_args()
    main(args)

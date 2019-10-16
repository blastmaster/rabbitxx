#! /usr/bin/env python3

import argparse

from analysis import experiment
from analysis import Filter
from analysis.metadata import make_creates, Create
from analysis.overlap import overlapping_writes, get_access_mappings, read_modify_write, read_after_write, SetAccessMap
from analysis.writemarker import write_marker_for_overlap, write_marker_for_concurrent_creates, write_marker_for_read_modify_write, write_marker_for_read_after_write, add_markers

import numpy as np

from typing import List

'''
Overlapping Accesses
    1. Filter cio sets for useless I/O such as `/proc` or `/sys`
    2. obtain remaining file names present
    3. recalculate the offset for every file and process <-
    4. obtain access ranges - intervall tree? Interval objects [start-offset:end-offset] (kind)
    5. compare if they overlap
'''


def read_accesses(setidx: int, filename: str, acc_map: List[SetAccessMap]):
    return [pfa.read_intervals for pfa in acc_map[setidx][filename] if pfa.has_reads()]


def write_accesses(setidx: int, filename: str, acc_map: List[SetAccessMap]):
    return [pfa.write_intervals for pfa in acc_map[setidx][filename] if pfa.has_writes()]


def report_overlap(ovlp) -> None:
    ''' Print simple report for an found overlap. '''

    print("Overlapping access found in CIO-Set Nr {}".format(ovlp.set_idx))
    print("{} {}".format(ovlp.first.iv, ovlp.second.iv))
    print("Filename: {}".format(ovlp.filename))
    print("Between process {0} and {1}".format(ovlp.first.process, ovlp.second.process))
    print("\n")


def report_concurrent_create(create: Create, experiment) -> None:
    ''' Print a simple report for concurrent creates in the same subdirectory. '''

    def get_data(exp, sidx, ridx):
        return exp.cio_sets[sidx].loc[ridx]

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


def report_distributed_read_after_write(raw) -> None:
    ''' Print a simple report for distributed read-after-write access on the same region of a file. '''

    print("Write from process {} on file {} @ {} -> read from process {} on file {} @ {}".format(
        raw.first.process, raw.first.filename, raw.first.iv,
        raw.second.process, raw.second.filename, raw.second.iv))


def report_experiment_info(experiment) -> None:
    ''' Print an overview report for the rabbitxx-experiment. '''

    print("Experiment:")
    print("Applicaton run duration: {:.2f}s".format(experiment.experiment_length()))
    print("Experiment duration: {:.2f}s".format(experiment.experiment_stats.duration))
    print("Number of Locations: {}".format(experiment.num_locations()))
    print("Trace file: {}".format(experiment.tracefile()))
    print("CIO-Sets:")
    print("Number of CIO-Sets: {}".format(experiment.num_cio_sets()))
    print("File Map:")
    for file, fs in experiment.file_map().items():
        print("\t{} : {}".format(file, fs))


def main(args) -> None:
    print("Start read experiment ...")
    exp = experiment.read_experiment(args.experiment_dir)
    if args.filter:
        exp.filter(Filter.file_filter)

    print("Start analysis ...")

    if args.info:
        report_experiment_info(exp)

    create_count = 0
    overlapping_count = 0
    rmw_count = 0
    raw_count = 0
    overlap_markers = set()
    rmw_markers = set()
    raw_markers = set()
    create_markers = set()

    #TODO: can we get some performance if we yield creates instead of create a list.
    if args.concurrent_creates:
        print("Analyze concurrent creates ...")
        for idx, create in enumerate([make_creates(sidx, cio_set) for sidx, cio_set in enumerate(exp.cio_sets)]):
            create_markers.update(write_marker_for_concurrent_creates(create, exp))
            report_concurrent_create(create, exp)
            create_count = idx + 1

    if args.overlap or args.read_modify_write or args.read_after_write:

        for acc_m in get_access_mappings(exp):
            print("checking set idx: {}".format(acc_m.set_index))
            for file, acc_l in acc_m.file_accesses.items():
                if args.overlap:
                    print("Analyze overlapping accesses ...")
                    for idx, ovlp in enumerate(overlapping_writes(file, acc_l)):
                        report_overlap(ovlp)
                        overlap_markers.update(write_marker_for_overlap(ovlp, exp.tracefile()))
                        overlapping_count = idx + 1
                if args.read_modify_write:
                    print("Analyze distributed read-modify-write ...")
                    if acc_m.num_processes(file) < 2:
                        continue
                    for idx, rmw in enumerate(read_modify_write(acc_l)):
                        rmw_markers.add(write_marker_for_read_modify_write(rmw, exp))
                        report_distributed_read_modify_write(rmw)
                        rmw_count = idx + 1
                if args.read_after_write:
                    print("Analyze distributed read-after-write ...")
                    if acc_m.num_processes(file) < 2:
                        continue
                    for idx, raw in enumerate(read_after_write(acc_l)):
                        raw_markers.add(write_marker_for_read_after_write(raw, exp))
                        report_distributed_read_after_write(raw)
                        raw_count = idx + 1

        print("{} markers for concurrent creates added.".format(create_count))
        print("{} markers for overlapping access added.".format(overlapping_count))
        print("{} markers for read modify write added.".format(rmw_count))
        print("{} markers for read after write added.".format(raw_count))

        print("{} markers for concurrent creates in set.".format(len(create_markers)))
        print("{} markers for overlapping access in set.".format(len(overlap_markers)))
        print("{} markers for read modify write in set.".format(len(rmw_markers)))
        print("{} markers for read after write in set.".format(len(raw_markers)))

        add_markers(create_markers)
        add_markers(overlap_markers)
        add_markers(rmw_markers)
        add_markers(raw_markers)

        print("Sum markers added {}".format(create_count + overlapping_count + rmw_count + raw_count))
    print("Done")


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
            '-f', '--filter',
            action='store_true',
            default=False,
            help='Enable experiment.FileFilter to filter files in /sys or /proc.',
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
    parser.add_argument(
            '-raw', '--read-after-write',
            action='store_true',
            default=False,
            help='Report distributed read-after-write access to the same region of a file.',
    )

    args = parser.parse_args()
    main(args)

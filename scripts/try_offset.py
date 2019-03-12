#! /usr/bin/env python3

from analysis import experiment
from analysis import Filter
from analysis.overlap import overlaps, get_access_mappings

'''
check if

writes overlaps with writes DONE
writes overlaps with reads
reads overlaps with reads
'''


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


def dump_set_files(set_files) -> None:
    ''' Dump file accessess of every set. '''

    for setidx, file_acc in set_files.items():
        print("Accessess in set {}".format(setidx))
        for filename, acc_list in file_acc.items():
            print("Accesses to {}".format(filename))
            for acc in acc_list:
                print(acc)



def main(path: str) -> None:
    exp = experiment.read_experiment(path)
    exp.filter(Filter.file_filter)
    set_files = get_access_mappings(exp)
    # dump_set_files(set_files)

    for acc_m in set_files:
        print("checking set idx: {}".format(acc_m.set_index))
        for file, acc_l in acc_m.file_accesses.items():
            for ovlp in overlaps(file, acc_l):
                report_overlap(ovlp)

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        sys.exit("Usage {} <path-to-experiment-dir>".format(sys.argv[0]))
    main(sys.argv[1])

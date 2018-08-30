#! /usr/bin/env python3

from analysis import experiment
from analysis.overlap import overlaps, get_access_mappings

'''
check if

writes overlaps with writes DONE
writes overlaps with reads
reads overlaps with reads
'''


def report_overlap(ovlp) -> None:

    print("Overlapping access found in CIO-Set Nr {}".format(ovlp.set_idx))
    print("{} {}".format(ovlp.first.iv, ovlp.second.iv))
    print("Filename: {}".format(ovlp.filename))
    print("Between process {0} and {1}".format(ovlp.first.process, ovlp.second.process))
    print("\n")


def main(path: str) -> None:
    exp = experiment.read_experiment(path)
    set_files = get_access_mappings(exp)

    for sidx, file_acc_d in set_files.items():
        print("checking set idx: {}".format(sidx))
        for file, acc_l in file_acc_d.items():
            for ovlp in overlaps(file, acc_l):
                report_overlap(ovlp)
            # print(list(overlaps(file, acc_l)))


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        sys.exit("Usage {} <path-to-experiment-dir>".format(sys.argv[0]))
    main(sys.argv[1])

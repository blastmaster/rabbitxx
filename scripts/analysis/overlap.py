
import numpy as np
import pandas as pd
from intervaltree import Interval, IntervalTree

from analysis import Filter
from analysis.accessTracking import get_file_acccess_per_process
from analysis import utils

from typing import List, Dict, Tuple, Iterable


def make_intervaltree(df: pd.DataFrame) -> IntervalTree:
    intervals = []
    if df.empty:
        raise Exception("Error! Try to make intervaltree from empty dataframe.")
    for idx, entry in df.iterrows():
        #if entry.response_size == entry.offset: # first operation
        start = entry['offset'] - entry['response_size']
        if start == entry['offset']:
            print("Emtpy interval! .. skip!")
            continue
        intervals.append(Interval(start, entry['offset'], (entry['kind'], idx)))
    return IntervalTree(intervals)


def make_read_intervaltree(df: pd.DataFrame) -> IntervalTree:
    ''' Create an IntervalTree of the write operations in the given CIO-Set. '''
    return make_intervaltree(df[df['kind'] == ' read'])

def make_write_intervaltree(df: pd.DataFrame) -> IntervalTree:
    ''' Create an IntervalTree of the read operations in the given CIO-Set. '''
    return make_intervaltree(df[df['kind'] == ' write'])

class ProcFileAccess:
    ''' Class to represent the file accesses of a process to a file in a CIO-Set. '''
    set_idx: int
    filename: str
    process: int
    #file_access_df: pd.DataFrame
    read_intervals: IntervalTree
    write_intervals: IntervalTree

    def __init__(self, idx: int, fname: str, pid: int, read_ivtree: IntervalTree, write_ivtree: IntervalTree) -> None:
        self.set_idx = idx
        self.file_name = fname
        self.process = pid
        self.read_intervals = read_ivtree if read_ivtree else None
        self.write_intervals = write_ivtree if write_ivtree else None

    def __str__(self) -> str:
        return "set idx: {} filename {} pid {} read_intervals {} write_intervals {}".format(
                self.set_idx, self.filename, self.process, self.read_intervals, self.write_intervals)

    def has_reads(self) -> bool:
        return self.read_intervals is not None

    def has_writes(self) -> bool:
        return self.write_intervals is not None

    @property
    def set_index(self) -> int:
        return self.set_idx

    @property
    def filename(self) -> str:
        return self.file_name

    @property
    def pid(self) -> int:
        return self.process

    # def overlapping_reads(self, other: ProcFileAccess):
    def overlapping_reads(self, other):
        ''' Check if `self.read_intervals` overlaps with some of the read intervals in `other`. '''
        for riv in other.read_intervals:
            oiv = self.read_intervals[riv.begin:riv.end]
            if oiv:
                print("Found overlapping read {} -> {}".format(oiv, riv))

    # def overlapping_writes(self, other: ProcFileAccess):
    def overlapping_writes(self, other):
        ''' Check if `self.write_intervals` overlaps with some of the write intervals in `other`. '''
        for wiv in other.write_intervals:
            oiv = self.write_intervals[wiv.begin:wiv.end]
            if oiv:
                print("Found overlapping write {} : {}".format(oiv, wiv))


class SetAccessMap:
    ''' Access map of a set. Provides a mapping from a filename to a list of
        ProcFileAccess objects representing the Access of each Process. '''

    # indice of cio-set
    setidx: int
    # mapping filename -> accesses per process
    file_accesses: Dict[str, List[ProcFileAccess]]

    def __init__(self, setidx: int, f_acc: Dict[str, List[ProcFileAccess]]) -> None:
        self.setidx = setidx
        self.file_accesses = f_acc

    def __getitem__(self, key: str) -> List[ProcFileAccess]:
        if key not in self.file_accesses.keys():
            raise KeyError("{} filename not found!".format(key))
        return self.file_accesses[key]

    def __str__(self) -> str:
        return "setidx {} file_accesses {}".format(self.setidx, self.file_accesses)

    @property
    def set_index(self) -> int:
        return self.setidx

    def files(self) -> List[str]:
        return list(self.file_accesses.keys())

    def processes(self, filename: str) -> List[int]:
        ''' Return a list of pids which accessing the given file. '''
        return [pfa.pid for pfa in self.file_accesses[filename]]

    def num_processes(self, filename: str) -> int:
        ''' Return the number of processes which accessing the given file. '''
        return len([pfa.pid for pfa in self.file_accesses[filename]])


# NOTE that the sets in experiment should be filtered already.
def get_access_mappings(exp) -> List[SetAccessMap]:
    ''' Returns an access mapping of each CIO-Set for its file accesses. '''

    set_files = [] # mapping setidx -> file_access dict
    for idx, file_list in exp.files().items():
        # for each set!
        file_accesses = dict() # mapping filename -> List[ProcFileAccess]
        for file in file_list:
            # for each file!
            acc_list = []
            for acc in get_file_acccess_per_process(exp.cio_sets[idx], file):
                # for each process!
                assert len(acc['filename'].unique()) == 1
                assert len(acc['pid'].unique()) == 1
                filename = acc['filename'].unique()[0]
                pid = acc['pid'].unique()[0]
                assert file == filename
                # print("processing process {} for file {}".format(pid, filename))
                kinds = acc['kind'].unique()
                w_it, r_it = None, None
                if ' write' in kinds:
                    # print('processing write')
                    w_it = make_write_intervaltree(acc)
                if ' read' in kinds:
                    # print('processing read')
                    r_it = make_read_intervaltree(acc)
                f_acc = ProcFileAccess(idx, filename, pid, r_it, w_it)
                acc_list.append(f_acc) # list of accesses for a file of each process
            file_accesses.update({file: acc_list})
        acc_m = SetAccessMap(idx, file_accesses)
        set_files.append(acc_m)

    return set_files


# same as above but for a given set instead of the whole experiment
# NOTE that the set should be filtered already.
def get_set_access_mapping(cio_set: pd.DataFrame, setidx: int) -> SetAccessMap:

    file_accesses = dict()
    for file in utils.get_files_in_set(cio_set):
        acc_list = []
        for acc in get_file_acccess_per_process(cio_set, file):
            assert len(acc['filename'].unique()) == 1
            assert len(acc['pid'].unique()) == 1
            filename = acc['filename'].unique()[0]
            assert file == filename

            pid = acc['pid'].unique()[0]
            kinds = acc['kind'].unique()
            w_it, r_it = None, None
            if ' write' in kinds:
                w_it = make_write_intervaltree(acc)
            if ' read' in kinds:
                r_it = make_read_intervaltree(acc)
            f_acc = ProcFileAccess(setidx, file, pid, r_it, w_it)
            acc_list.append(f_acc)
        file_accesses.update({file: acc_list})

    return SetAccessMap(setidx, file_accesses)


class AccInterval:
    iv: Interval
    process: int
    filename: str
    setidx: int

    def __init__(self, iv: Interval, pid: int, fn: str, idx: int) -> None:
        self.iv = iv
        self.process = pid
        self.filename = fn
        self.setidx = idx
        #TODO row index

    def __str__(self) -> str:
        return "AccInterval {} pid {} file {} set {}".format(self.iv, self.process, self.filename, self.setidx)


class Overlap:
    first: AccInterval
    second: AccInterval

    def __init__(self, first: AccInterval, second: AccInterval) -> None:
        assert first.setidx == second.setidx, "Overlaps must occur in the same set!"
        assert first.filename == second.filename, "Overlaps must occur in the same File!"
        self.first = first
        self.second = second

    def __str__(self) -> str:
        return "Overlap {} : {}".format(self.first, self.second)

    @property
    def set_idx(self) -> int:
        return self.first.setidx

    @property
    def filename(self) -> str:
        return self.first.filename

    @property 
    def processes(self) -> Tuple[int, int]:
        return (self.first.process, self.second.process)


def is_overlapping(head: ProcFileAccess, tail: List[ProcFileAccess]) -> Iterable:

    #print('tail len: {}'.format(len(tail)))
    for cmp_acc in tail:
        #print('compare access of process {} with accesses of process {}'.format(head.process, cmp_acc.process))
        assert cmp_acc != head # do not compare with yourself!
        if cmp_acc.has_writes() and head.has_writes():
            for wiv in cmp_acc.write_intervals:
                oiv = head.write_intervals[wiv.begin:wiv.end]
                if oiv:
                    #print('found overlapping write: {} -> with {}'.format(oiv, wiv))
                    first = AccInterval(oiv, head.process, head.filename, head.set_idx)
                    second = AccInterval(wiv, cmp_acc.process, cmp_acc.filename, cmp_acc.set_idx)
                    ov = Overlap(first, second)
                    yield ov
        if cmp_acc.has_reads() and head.has_reads():
            for riv in cmp_acc.read_intervals:
                oiv = head.read_intervals[riv.begin:riv.end]
                if oiv:
                    first = AccInterval(oiv, head.process, head.filename, head.set_idx)
                    second = AccInterval(riv, cmp_acc.process, cmp_acc.filename, cmp_acc.set_idx)
                    ov = Overlap(first, second)
                    yield ov


def overlaps(filename: str, acc_l: List[ProcFileAccess]) -> Iterable:

    for i, head in enumerate(acc_l):
        yield from is_overlapping(head, acc_l[i+1:])


    for i, head in enumerate(acc_l):
def read_modify_write(acc_l: List[ProcFileAccess]) -> Iterable:

    for idx, head in enumerate(acc_l):
        tail = acc_l[:idx] + acc_l[(idx + 1):]
        yield from rmw(head, tail)


def rmw(acc: ProcFileAccess, tail: List[ProcFileAccess]):
    ''' check if any read interval of acc is written in any write interval in tail. '''
    if acc.has_reads():
        for riv in acc.read_intervals:
            for other in tail:
                if other.has_writes():
                    oiv = other.write_intervals[riv.begin:riv.end]
                    if oiv:
                        first = AccInterval(riv, acc.process, acc.filename, acc.set_idx)
                        second = AccInterval(oiv, other.process, other.filename, other.set_idx)
                        yield Overlap(first, second)


def read_after_write(acc_l: List[ProcFileAccess]) -> Iterable:

    for idx, head in enumerate(acc_l):
        tail = acc_l[:idx] + acc_l[(idx + 1):]
        yield from raw(head, tail)


def raw(acc: ProcFileAccess, tail: List[ProcFileAccess]):
    ''' distributed read-after-write '''

    if acc.has_writes():
        for wiv in acc.write_intervals: # check all write intervals
            for other in tail:
                if other.has_reads():
                    overlapping_interval = other.read_intervals[wiv.begin:wiv.end] # against all read intervals
                    if overlapping_interval:
                        first = AccInterval(wiv, acc.process, acc.filename, acc.set_idx)
                        second = AccInterval(overlapping_interval, other.process, other.filename, other.set_idx)
                        yield Overlap(first, second)


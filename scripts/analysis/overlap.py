
import numpy as np
import pandas as pd
from intervaltree import Interval, IntervalTree

from analysis import Filter
from analysis.accessTracking import recalculate_offset_of_set

from typing import List, Dict, Tuple, Iterable


def get_files_per_set(exp):
    ''' Get an Experiment and filter files. Returns a dict matching a set index
        to a list of accessed file names within this set.
    '''
    files_per_set = dict()
    for idx, cs in Filter.apply_file_filter(exp): #TODO need user provided filter list
        files_per_set.update({idx: np.unique(cs['filename'].values)})
    return files_per_set



def set_accesses(exp) -> Dict[int, List[Tuple[str, Iterable]]]:

    d = dict()
    files_per_set = get_files_per_set(exp)
    for idx, files in files_per_set.items():
        cur_set = exp.cio_sets[idx]
        # for the current set calculate offset of every touched file
        dfl = []
        for file in files:
            group = recalculate_offset_of_set(cur_set, file)
            #off_df = pd.concat(group) # merge the different processes into a single dataframe, maybe set index to timestamp for ordering?
            dfl.append((file, group))
        d.update({idx: dfl})
    return d


def make_intervaltree(df: pd.DataFrame) -> IntervalTree:
    intervals = []
    if df.empty:
        raise Exception("Error! Try to make intervaltree from empty dataframe.")
    for idx, entry in df.iterrows():
        #if entry.response_size == entry.offset: # first operation
        start = entry['offset'] - entry['response_size'] # TODO
        intervals.append(Interval(start, entry['offset'], entry['kind']))
    return IntervalTree(intervals)


def make_read_intervaltree(df: pd.DataFrame) -> IntervalTree:
    return make_intervaltree(df[df['kind'] == ' read'])

def make_write_intervaltree(df: pd.DataFrame) -> IntervalTree:
    return make_intervaltree(df[df['kind'] == ' write'])

class ProcFileAccess:
    set_idx: int
    filename: str
    process: int
    #file_access_df: pd.DataFrame
    read_intervals: IntervalTree
    write_intervals: IntervalTree

    def __init__(self, idx: int, fname: str, pid: int, read_ivtree: IntervalTree, write_ivtree: IntervalTree) -> None:
        self.set_idx = idx
        self.filename = fname
        self.process = pid
        self.read_intervals = read_ivtree if read_ivtree else None
        self.write_intervals = write_ivtree if write_ivtree else None

    def has_reads(self) -> bool:
        return self.read_intervals is not None

    def has_writes(self) -> bool:
        return self.write_intervals is not None

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


def get_access_mappings(exp) -> Dict[int, List[ProcFileAccess]]:

    set_files = dict()
    for idx, file_list in set_accesses(exp).items():
        # for each set!
        file_accesses = dict()
        print("processing set {}".format(idx))
        for file, file_gen in file_list: # file_list yields (filename, generator)
            # for each file!
            acc_list = []
            for i, acc in enumerate(file_gen):
                # for each process!
                assert len(acc['filename'].unique()) == 1
                assert len(acc['pid'].unique()) == 1
                pid = acc['pid'].unique()[0]
                filename = acc['filename'].unique()[0]
                assert file == filename
                print("processing process {} for file {}".format(pid, filename))
                kinds = acc['kind'].unique()
                w_it, r_it = None, None
                if ' write' in kinds:
                    print('processing write')
                    w_it = make_write_intervaltree(acc)
                if ' read' in kinds:
                    print('processing read')
                    r_it = make_read_intervaltree(acc)
                f_acc = ProcFileAccess(idx, filename, pid, r_it, w_it)
                acc_list.append(f_acc) # list of accesses for a file of each process
            file_accesses.update({file: acc_list})
        set_files.update({idx: file_accesses})

    return set_files


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
    ''' TODO at the moment looks only for writes! '''

    print('tail len: {}'.format(len(tail)))
    for cmp_acc in tail:
        print('compare access of process {} with accesses of process {}'.format(head.process, cmp_acc.process))
        assert cmp_acc != head # do not compare with yourself!
        if cmp_acc.has_writes():
            for wiv in cmp_acc.write_intervals:
                oiv = head.write_intervals[wiv.begin:wiv.end]
                if oiv:
                    #print('found overlapping write: {} -> with {}'.format(oiv, wiv))
                    first = AccInterval(oiv, head.process, head.filename, head.set_idx)
                    second = AccInterval(wiv, cmp_acc.process, cmp_acc.filename, cmp_acc.set_idx)
                    ov = Overlap(first, second)
                    yield ov


def overlaps(filename: str, acc_l: List[ProcFileAccess]) -> Iterable:

    print('looking for overlaps in {}'.format(filename))
    for i, head in enumerate(acc_l):
        yield from is_overlapping(head, acc_l[i+1:])



import pandas as pd
import numpy as np

from typing import List, Dict

class Create:
    set_index: int
    row_indices: List[int]
    creates: Dict[str, int] # mapping directory name -> number of creates

    def __init__(self, sidx: int, ridx: List[int], files: List[str], counts: List[int]) -> None:
        self.set_index = sidx
        self.row_indices = ridx
        self.creates = dict(zip(files, counts))

    def __str__(self) -> str:
        return "set index: {} creates {}".format(self.set_index, self.creates)

    # FIXME does the opposite but don't know why?!?
    # def empty(self) -> bool:
        # return bool(self.creates)

def concurrent_creates(cio_set: pd.DataFrame, directory=None, with_indices=False):
    ''' Return an array of directories where parallel creates happen and an Array with the number of parallel creates.'''
    create_ops = cio_set[cio_set['kind'] == ' create']

    sub_dir_crts = [p[:p.rindex('/')] for p in create_ops['filename'].tolist()]
    files, counts = np.unique(sub_dir_crts, False, False, True)
    if with_indices:
        indices = cio_set.index[cio_set['kind'] == ' create'].tolist()
        return files, counts, indices
    return files, counts


def make_creates(set_idx: int, cio_set: pd.DataFrame, directory=None):

    files, counts, row_indices = concurrent_creates(cio_set, with_indices=True)
    return Create(set_idx, row_indices, files, counts)

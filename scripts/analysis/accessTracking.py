import pandas as pd


class OffsetTracker:
    value: int = 0

    def __init__(self, initial: int=0) -> None:
        self.value = initial

    def __repr__(self) -> str:
        return 'OffsetTracker value={}'.format(self.value)

    @property
    def offset(self) -> int:
        return self.value

    @offset.setter
    def offset(self, value: int) -> None:
        self.value = value


def track_file_offsets(row, **kwds):

    tracker = kwds['tracker']

    if row['kind'] == ' create':
        # print('create')
        assert tracker.offset == 0, "Initial offset unequal to zero on create operation"

    elif row['kind'] == ' seek':
        # print('seek')
        tracker.offset += row['response_size']

    elif row['kind'] == ' write' or row['kind'] == ' read':
        # print('read/write')
        tracker.offset += row['response_size']

    elif row['kind'] == ' close_or_delete':
        # return current offset but reset tracker!
        tmp = tracker.offset
        tracker.offset = 0
        return tmp

    return int(tracker.offset)


def prefix_file_filter(file_map, prefix: str):
    return {file: fs for file, fs in file_map.items() if file.startswith(prefix)}


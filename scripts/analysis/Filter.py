import pandas as pd

'''
    Filter functions can be chained together via `pipe`.
'''

# a pre-defined filter ready to use
def apply_file_filter(exp):
    ''' Apply a set of filter methods on the sets and override the experiment results by that.
        Returns a generator which produces a tuple of (set_idx, cio_set).
    '''
    #FIXME might be better if we can pass a list of filters to a function and this function applies them all.
    exp.cio_sets = [
        cio_set.pipe(virtual_fs_filter)
            .pipe(cgroup_fs_filter)
            .pipe(sw_filter)
            .pipe(stdout_filter)
            .pipe(dev_fs_filter)
            .pipe(etc_filter)
            .pipe(filename_filter, fname=' /') for cio_set in exp.cio_sets]
    for idx, cio_set in enumerate(exp.cio_sets):
        if cio_set.empty:
            print("Filter empty set nr: {}".format(idx + 1))
        else:
            yield idx, cio_set


def file_filter(cio_set: pd.DataFrame) -> pd.DataFrame:
    # TODO formatting
    return cio_set.pipe(virtual_fs_filter).pipe(cgroup_fs_filter).pipe(sw_filter).pipe(stdout_filter).pipe(dev_fs_filter).pipe(etc_filter).pipe(run_filter).pipe(filename_filter, fname=' /')


''' PREFIX FILTERS
    Prefix filters checks if a path/filename starts with a known substring.
'''

def stdout_filter(df):
    ''' Filter access to stdout '''
    return df[~(df['filename'].str.startswith(' stdout') | df['filename'].str.startswith(' STDOUT_FILENO'))]

def virtual_fs_filter(df):
    ''' Filter access to virtual file systems like `/sys` and `/proc` '''
    return df[~(df['filename'].str.startswith(' /sys') | df['filename'].str.startswith(' /proc'))]

def dev_fs_filter(df):
    return df[~(df['filename'].str.startswith(' /dev'))]

def cgroup_fs_filter(df):
    ''' Filter access to `/cgroup` file system. '''
    return df[~(df['filename'].str.startswith(' /cgroup'))]

def etc_filter(df):
    ''' Filter access to `/etc` '''
    return df[~(df['filename'].str.startswith(' /etc'))]

def run_filter(df):
    ''' Filter access to `/run` '''
    return df[~(df['filename'].str.startswith(' /run'))]

def sw_filter(df):
    ''' Filter access to `/sw`. NOTE This is Taurus specific. '''
    return df[~(df['filename'].str.startswith(' /sw') | df['filename'].str.startswith(' /software'))]


''' EXACT FILTER
    The path/filename to filter for must match the exactly filename.
'''

def filename_filter(df, fname=''):
    ''' Filter accessess of exact filenames given by fname '''
    if fname is None or len(fname) == 0:
        raise ValueError('No file name given to filter for!')
    return df[~(df['filename'] == fname)]


''' KIND FILTERS '''

def read_kinds(df):
    ''' Filter any kinds of operation except `read` operations. '''
    return df[(df['kind'] == ' read')]

def write_kinds(df):
    ''' Filter any kinds of operation except `write` operations. '''
    return df[(df['kind'] == ' write')]

def seek_kinds(df):
    ''' Filter any kinds of operation except `seek` operations. '''
    return df[(df['kind'] == ' seek')]

def flush_kinds(df):
    ''' Filter any kinds of operation except `flush` operations. '''
    return df[(df['kind'] == ' flush')]

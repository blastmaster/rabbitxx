'''
    Filter functions can be chained together via `pipe`.
'''

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

def sw_filter(df):
    ''' Filter access to `/sw`. NOTE This is Taurus specific. '''
    return df[~(df['filename'].str.startswith(' /sw'))]


''' EXACT FILTER
    The path/filename to filter for must match the exactly filename.
'''

def filename_filter(df, fname=''):
    ''' Filter accessess of exact filenames given by fname '''
    if fname is None or len(fname) == 0:
        raise ValueError('No file name given to filter for!')
    return df[~(df['filename'] == fname)]


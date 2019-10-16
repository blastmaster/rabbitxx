
import subprocess
from typing import List

'''
./run_analysis.py -ov ../test/rabbitxx-overlapping_write

--add-def <GROUP> <CATEGORY> <SEVERITY> Add a new marker definition.
--add <GROUP> <CATEGORY> <TIME> <SCOPE> <TEXT> Add a marker to an existing definition.
'''

class Marker:
    group: str
    category: str
    time: str
    scope: str
    text: str
    trcfile: str

    def __init__(self, group, category, time, scope, text, trcfile):
        self.group = group
        self.category = category
        self.time = time
        self.scope = scope
        self.text = text
        self.trcfile = trcfile

    def __hash__(self):
        return hash((self.group, self.category, self.category,
            self.time, self.scope, self.text, self.trcfile))

    def __eq__(self, other):
        return (self.group == other.group 
                and self.category == other.category
                and self.time == other.time
                and self.scope == other.scope
                and self.text == other.text
                and self.trcfile == other.trcfile)


def run_marker_tool(args: List[str]):
    ''' execute the otf2-marker tool with corresponding arguments. '''
    cmd = ['otf2-marker'] + args
    print("DEBUG execute: {}".format(cmd))
    subprocess.run(cmd, check=True)


def add_marker_def(group: str, category: str, severity: str, trcfile: str):
    ''' add the marker definition
        --add-def <GROUP> <CATEGORY> <SEVERITY> Add a new marker definition.
    '''
    if severity.upper() not in ['NONE', 'LOW', 'MEDIUM', 'HIGH']:
        raise ValueError("severity must be none, low, medium or high but is {}".format(severity))
    args = ['--add-def', group, category, severity, trcfile]
    run_marker_tool(args)


def add_markers(marker_list: set) -> None:
    ''' Get a set of marker objects and add them all. '''
    for marker in marker_list:
        add_marker(marker)


def add_marker(marker: Marker):
    ''' add the marker
        --add <GROUP> <CATEGORY> <TIME> <SCOPE> <TEXT> Add a marker to an existing definition.
	Scope:
	GLOBAL
	LOCATION:<LOCATION-REF>
	LOCATION_GROUP:<LOCATION-GROUP-REF>
	SYSTEM_TREE_NODE:<SYSTEM-TREE-NODE-REF>
	GROUP:<GROUP-REF>
	COMM:<COMMUNICATOR-REF>
    '''
    args = ['--add', marker.group, marker.category, marker.time, marker.scope, marker.text, marker.trcfile]
    run_marker_tool(args)


#def write_marker_for_overlap(ovlp: Overlap, tracefile: str):
def write_marker_for_overlap(ovlp, tracefile: str):

    group = 'rabbitxx'
    category = 'overlapping access'
    #scope location:0
    # TODO why is `iv` a set??? not just interval?
    timestamp = next(iter(ovlp.first.iv)).data[-1]
    timestamp2 = ovlp.second.iv.data[-1]

    scope = 'LOCATION:{}'.format(ovlp.first.process)
    scope2 = 'LOCATION:{}'.format(ovlp.second.process)

    text = "overlapping access:\n\
            {} {}\n\
            Filename: {}\n\
            Between process {} and {}".format(
                    ovlp.first.iv, ovlp.second.iv,
                    ovlp.filename,
                    ovlp.first.process, ovlp.second.process)

    try:
        add_marker_def(group, category, 'high', tracefile)
    except:
        pass

    return set((Marker(group, category, str(timestamp), scope, text, tracefile),
        Marker(group, category, str(timestamp2), scope2, text, tracefile)))


def write_marker_for_concurrent_creates(create, exp):

    group = 'rabbitxx'
    category = 'concurrent creates'
    cio_set = exp.cio_sets[create.set_index]
    marker_set = set()

    try:
        add_marker_def(group, category, 'high', exp.tracefile())
    except:
        pass

    for idx, row in cio_set.loc[create.row_indices].iterrows():
        timestamp = row.timestamp
        scope = 'LOCATION:{}'.format(row.pid)
        text = 'concurrent create\n\
                filename: {}\n'.format(row.filename)

        marker_set.add(Marker(group, category, str(timestamp), scope, text, exp.tracefile()))
    return marker_set


def write_marker_for_read_modify_write(rmw, exp):

    group = 'rabbitxx'
    category = 'read-modify-write'
    timestamp = next(iter(rmw.first.iv)).data[-1]
    scope = 'LOCATION:{}'.format(rmw.first.process)
    text = 'read-modify-write:\n\
            {} {}\n\
            Filename: {}\n\
            Between process {} and {}'.format(rmw.first.iv, rmw.second.iv,
                    rmw.filename(), rmw.first.process, rmw.second.process)

    try:
        add_marker_def(group, category, 'high', exp.tracefile())
    except:
        pass

    return Marker(group, category, str(timestamp), scope, text, exp.tracefile())


def write_marker_for_read_after_write(raw, exp):

    group = 'rabbitxx'
    category = 'read-modify-write'
    timestamp = next(iter(raw.first.iv)).data[-1]
    scope = 'LOCATION:{}'.format(raw.first.process)
    text = 'read-modify-write:\n\
            {} {}\n\
            Filename: {}\n\
            Between process {} and {}'.format(raw.first.iv, raw.second.iv,
                    raw.filename(), raw.first.process, raw.second.process)

    try:
        add_marker_def(group, category, 'high', exp.tracefile())
    except:
        pass

    return Marker(group, category, str(timestamp), scope, text, exp.tracefile())

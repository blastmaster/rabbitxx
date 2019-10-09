
import subprocess
from typing import List

'''
./run_analysis.py -ov ../test/rabbitxx-overlapping_write

--add-def <GROUP> <CATEGORY> <SEVERITY> Add a new marker definition.
--add <GROUP> <CATEGORY> <TIME> <SCOPE> <TEXT> Add a marker to an existing definition.
'''

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


def add_marker(group: str, category: str, time, scope, text, trcfile):
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
    args = ['--add', group, category, time, scope, text, trcfile]
    run_marker_tool(args)

#TODO add tracefile as argument to marker tool
#def write_marker_for_overlap(ovlp: Overlap, tracefile: str):
def write_marker_for_overlap(ovlp, tracefile: str):

    group = 'rabbitxx'
    category = 'overlapping access'
    #scope location:0
    # TODO why is `iv` a set??? not just interval?
    timestamp = next(iter(ovlp.first.iv)).data[-1]
    scope = 'LOCATION:{}'.format(ovlp.first.process)
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
    add_marker(group, category, str(timestamp), scope, text, tracefile)

def write_marker_for_concurrent_creates(create, exp):

    group = 'rabbitxx'
    category = 'concurrent creates'
    cio_set = exp.cio_sets[create.set_index]

    try:
        add_marker_def(group, category, 'high', exp.tracefile())
    except:
        pass

    for idx, row in cio_set.loc[create.row_indices].iterrows():
        timestamp = row.timestamp
        scope = 'LOCATION:{}'.format(row.pid)
        text = 'concurrent create\n\
                filename: {}\n'.format(row.filename)

        add_marker(group, category, str(timestamp), scope, text, exp.tracefile())

def write_marker_for_read_modify_write(rmw, exp):

    group = 'rabbitxx'
    category = 'read-modify-write'
    timestamp = next(iter(rmw.first.process)).data[-1]
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

    add_marker(group, category, str(timestamp), scope, text, exp.tracefile())


def write_marker_for_read_after_write(raw, exp):

    group = 'rabbitxx'
    category = 'read-modify-write'
    timestamp = next(iter(raw.first.process)).data[-1]
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

    add_marker(group, category, str(timestamp), scope, text, exp.tracefile())

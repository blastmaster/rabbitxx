import os
import json
import pandas as pd

class ExperimentStats:
    def __init__(self, graph_stats, cio_stats, pio_stats, **kw):
        self.duration = kw['Experiment_Duration']
        self.tracefile = kw['Tracefile']
        self.clock_properties = kw['Clock Properties']
        self.file_map = kw['File Map']
        self.graph_stats = graph_stats
        self.cio_stats = cio_stats
        self.pio_stats = pio_stats


class CIOStats:
    def __init__(self, **kw):
        self.build_time = kw['Build time'] #FIXME
        self.number_of_sets = kw['Number of CIO-Sets']
        self.set_durations = kw['Set durations']


class PIOStats:
    def __init__(self, **kw):
        self.build_time = kw['Build time'] #FIXME
        self.number_of_sets = kw['Number of PIO-Sets']


class GraphStats:
    def __init__(self, **kw):
        self.build_time = kw['Build time']
        self.num_edges = kw['Number of Edges']
        self.num_vertices = kw['Number of Vertices']


class Experiment:
    def __init__(self, cio_sets, pio_sets, experiment_stats):
        self.cio_sets = cio_sets
        self.pio_sets = pio_sets
        self.experiment_stats = experiment_stats

    def experiment_length(self) -> float:
        ''' Return the length of the application run in seconds. '''
        return self.experiment_stats.clock_properties['Length'] / self.experiment_stats.clock_properties['Ticks per Seconds']


def find_experiment_files(base_path):

    summary_file = ''
    cio_set_files = []
    pio_set_files = {}

    for root, dirs, files in os.walk(base_path):
        if root == base_path:
            for file in files:
                if file.endswith('.json'): #FIXME just use json file, even a csv version will be produced.
                    summary_file = os.path.join(root, file)
            #summary_file = os.path.join(root, files[0])
        if os.path.basename(root) == 'cio-sets':
            cio_set_files = [os.path.join(root, f) for f in files]
        if os.path.basename(root) == 'pio-sets':
            for p_dir in [os.path.join(root, d) for d in dirs]:
                for p_root, _, p_files in os.walk(p_dir):
                    pio_set_files[os.path.basename(p_root)] = [os.path.join(p_root, f) for f in p_files]

    return {"summary": summary_file,
            "cio_sets": cio_set_files,
            "pio_sets": pio_set_files}


def read_summary(filename):
    ''' Read summary json file, return python object. '''
    with open(filename, 'r') as f:
        summary = json.load(f)
        return summary


def read_set_csv(cio_set_files):
    ''' Read list of set files, return list of pandas.DataFrame. '''
    return [pd.read_csv(file) for file in cio_set_files]


def read_pio_set_csv(pio_set_files):

    return {k: read_set_csv(v) for k, v in pio_set_files.items()}


def read_experiment(path):

    fd = find_experiment_files(path)

    cio_sets = read_set_csv(fd['cio_sets']) if len(fd['cio_sets']) != 0 else []
    pio_sets = read_pio_set_csv(fd['pio_sets']) if len(fd['pio_sets']) != 0 else {}

    if fd['summary'] is not None and len(fd['summary']) != 0:
        summary = read_summary(fd['summary'])
        graph_st = GraphStats(**summary['Graph Stats'])
        cio_st = CIOStats(**summary['CIO Stats'])
        pio_st = PIOStats(**summary['PIO Stats'])
        exp_st = ExperimentStats(graph_st, cio_st, pio_st, **summary)
        return Experiment(cio_sets, pio_sets, exp_st)
    #TODO: else?!? without experiment


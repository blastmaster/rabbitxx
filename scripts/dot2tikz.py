#! /usr/bin/env python3

import sys
import os
import re
from string import Template
from enum import Enum
from collections import namedtuple

import pprint


TIKZ_PREAMBLE = Template('''\\begin{tikzpicture}[
    vertex/.style={
        scale=0.8,
        rounded rectangle,
        draw=black,
        thin},
    synth/.style={
        vertex,
        fill=gray!20},
    sync/.style={
        vertex,
        fill=green!40},
    io/.style={
        vertex,
        fill=red!40},
    ]''')

TIKZ_END = Template('''\end{tikzpicture}''')

class VertexKind(Enum):
    SYNTH = 0
    SYNC = 1
    IO = 2

def vertex_kind_to_string(kind):

    if kind == VertexKind.SYNTH:
        return "synth"
    if kind == VertexKind.SYNC:
        return "sync"
    if kind == VertexKind.IO:
        return "io"

def get_num_procs(nodes):
    max_pid = 0
    for _, node in nodes.items():
        if node.pid:
            max_pid = max(max_pid, node.pid)
    return max_pid + 1

class Node:
    def __init__(self, id, region, type, pid=None):
        self.id = int(id)
        self.region = re.sub("_", "", region)
        self.type = type
        if pid:
            self.pid = int(pid)
        else:
            self.pid = pid
        self.xpos = 1.0
        self.ypos = -1.0

    def node_name(self):
        if self.type == VertexKind.SYNTH:
            return self.region
        return self.region + str(self.id)

    def to_tikz_node_definition(self, num_procs):

        type_str = vertex_kind_to_string(self.type)
        positionX = 0.0
        if self.type != VertexKind.SYNTH:
            print(self.type)
            print(self.region)
            print(self.pid)
            positionX = (self.pid + 1) * num_procs * self.xpos

        positionY=0
        return "\\node[{type}] ({name}) at ({ypos}, {xpos}) {{{region}}};".format(
                type=type_str,
                name=self.node_name(),
                ypos=positionX,
                xpos=positionY,
                region=self.region)

Edge = namedtuple('Edge', ['src', 'trg'])

class Graph:

    def __init__(self, nodes=dict(), edges=[]):
        self.nodes = nodes
        self.edges = edges



def is_node(line):
    # example for node definition
    # 1[label="# 1 MPI_Send @ 1", color=green];
    return re.match('^\d+\[', line) is not None


def is_edge(line):
    # example for edge definition
    # 1->2 ;
    return re.match('^\d+->\d+', line) is not None


def parse_node(line):

    def parse_type(type_str):
        idx = type_str.find('=') + 1
        color = type_str[idx:]
        if color == 'green':
            return VertexKind.SYNC
        if color == 'red':
            return VertexKind.IO
        if color == 'gray':
            return VertexKind.SYNTH

    def parse_label(label_str):
        idx = label_str.find("=") + 2
        return label_str[idx:-2].strip()

    # synth events have no process therefor just two elements in the list
    match = re.match('(\d+)\[([^\]]+)\]', line)
    if match:
        id = int(match.group(1))
        lst = match.group(2).split()
        v_type = parse_type(lst.pop())
        if v_type == VertexKind.SYNTH:
            region = parse_label(lst[0])
            return Node(id, region, v_type)
        tail_lst = lst[2:]
        pid_str = tail_lst.pop()[:-2]
        region = tail_lst[0]
        return Node(id, region, v_type, int(pid_str))
    else:
        raise ValueError("Cannot parse line {}".format(line))


def parse_edge(line):

    sidx = line.find('->')
    eidx = sidx + 2
    src_id = int(line[:sidx])
    trg_id = int(line[eidx:-1].strip())
    return Edge(src_id, trg_id)


def line_generator(filename):

    with open(filename) as f:
        for line in f.readlines():
            yield line.strip()


def read_dot(filename):

    ln_gen = line_generator(filename)
    nodes = dict()
    edges = []
    for line in ln_gen:
        if is_node(line):
            node = parse_node(line)
            nodes[node.id] = node
        if is_edge(line):
            edge = parse_edge(line)
            edges.append(edge)

    return Graph(nodes, edges)


def write_graph_to_tikz(graph, tikzfile, num_procs):

    def edge_to_tikz(src_node_name, trg_node_name):

        return "\draw[->] ({0}) to ({1});".format(
                src_node_name,
                trg_node_name)

    with open(tikzfile, 'w+') as fout:
        print(TIKZ_PREAMBLE.substitute(), file=fout)
        for _, node in graph.nodes.items():
            tikz_node_def = node.to_tikz_node_definition(num_procs)
            print(tikz_node_def, file=fout)

        print("\n\n", file=fout)

        for edge in graph.edges:
            src_node_name = graph.nodes[edge.src].node_name()
            trg_node_name = graph.nodes[edge.trg].node_name()
            tikz_edge_def = edge_to_tikz(src_node_name, trg_node_name)
            print(tikz_edge_def, file=fout)

        print("\n\n", file=fout)
        print(TIKZ_END.substitute(), file=fout)



def main(args):

    if len(args) < 2:
        print("Error, usage {} <dot-file>".format(args[0]))
        sys.exit()

    filename = args[1]
    basename = os.path.basename(filename)
    tikzfile = os.path.splitext(basename)[0] + ".tex"

    graph = read_dot(filename)
    num_procs = get_num_procs(graph.nodes)
    print("num procs: {}".format(num_procs))
    write_graph_to_tikz(graph, tikzfile, num_procs)


if __name__ == '__main__':
    main(sys.argv)

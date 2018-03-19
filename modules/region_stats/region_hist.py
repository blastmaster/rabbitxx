#! /usr/bin/env python3

import matplotlib
import matplotlib.pyplot as plt
import numpy as np


def read_file(filename):

    data = np.genfromtxt(filename,
            names=['regions'], dtype=None)
    return data


data = read_file(filename)
region_names = data['regions']
regions, unique_regions, regions_inv, regions_count = np.unique(region_names,
        True, True, True)

fig, ax = plt.subplots()
ind = np.arange(regions.size)
width=0.35
ax.barh(regions, regions_count)
plt.ylabel('Function names')
plt.xlabel('Number of Invocatios')
plt.grid(True)
plt.show()

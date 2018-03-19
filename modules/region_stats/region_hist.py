#! /usr/bin/env python3

import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

fig_width = 10.82
fig_height = 6.1
plt.rcParams['figure.figsize'] = fig_width, fig_height

def read_file(filename):

    data = np.genfromtxt(filename,
            names=['regions'], dtype=None)
    return data


filename = None
if len(sys.argv) < 2 and not filename:
    print("Error no Input file")
    sys.exit(1)
else:
    filename = sys.argv[1]

data = read_file(filename)
region_names = data['regions']
regions, unique_regions, regions_inv, regions_count = np.unique(region_names,
        True, True, True)

fig, ax = plt.subplots()
ind = np.arange(regions.size)
width=0.35
# ax.barh(regions, regions_count)
ax.bar(regions, regions_count)
plt.ylabel('Function names')
plt.xlabel('Number of Invocations')

rects = ax.patches
for rect, noi in zip(rects, regions_count):
    height = rect.get_height()
    width = rect.get_width()
    # ax.text(rect.get_y() + rect.get_width() / 2, 5+rect.get_x(), noi,
    # ax.text(rect.get_y() + rect.get_width() / 2, 5+rect.get_x(), noi)
            # ha='center', va='bottom')
    ax.text(rect.get_x() + rect.get_width() / 2, height + 5, noi,
            ha='center', va='bottom')

ax.tick_params(axis='x', labelsize=12)
ax.tick_params(axis='y', labelsize=12)
# plt.grid(True)
# plt.show()
fig.savefig('regionstats.png', bbox_inches='tight')

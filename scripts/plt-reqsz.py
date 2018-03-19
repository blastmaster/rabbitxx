#! /usr/bin/env python3

import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def read_file(filename):

    pconv = lambda s: int(s)
    kbconv = lambda s: int(s) / 1000
    dconv = lambda s: int(s[:-2]) / 1000000
    data = np.genfromtxt(filename, skip_header=1,
            converters={0: pconv, 1: kbconv, 2: dconv},
            names=['process', 'reqsz', 'duration', 'region'], dtype=None)
    return data


if len(sys.argv) < 2:
    print("Error input file needed")
    sys.exit(1)

filename = sys.argv[1]
# filename = "/home/soeste/code/rabbitxx/modules/request_sizes/filtered_madbench2-20170613_1523_40520294257341.txt"
data = read_file(filename)
request_size = data['reqsz']
reqsz, unique_req_sz_idx, reqsz_inv, reqsz_count = np.unique(request_size, True, True, True)
print(reqsz)
print(reqsz_count)

fig, ax = plt.subplots()
ind = N = np.arange(reqsz.size)
print(ind)
width = 0.35
plt.xlim(width-len(ind), len(ind)+width)
#plt.xlim(reqsz.min(), reqsz.max())
plt.title('Request sizes', fontsize=20)
ax.bar(ind, reqsz_count, width)
ax.xaxis.set_ticklabels(reqsz, rotation=45)
ax.xaxis.set_ticks(ind)
plt.ylabel('Count', fontsize=16)
plt.xlabel('Request Sizes in Kilobytes', fontsize=16)
plt.show()


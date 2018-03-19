#! /usr/bin/env python3

import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np

def read_file(filename):

    pconv = lambda s: int(s)
    dconv = lambda s: int(s[:-2]) / 1000000
    data = np.genfromtxt(filename, skip_header=1,
            converters={0: pconv, 1: pconv, 2: dconv},
            names=['process', 'reqsz', 'duration'], dtype=None)
    return data


# if len(sys.argv) < 2:
    # print("Error input file needed")
    # sys.exit(1)

# filename = sys.argv[1]
filename = "./madbench2-20170613_1523_40520294257341.txt"
data = read_file(filename)
request_size = data['reqsz']
reqsz, unique_req_sz_idx, reqsz_inv, reqsz_count = np.unique(request_size, True, True, True)
print(reqsz)
print(reqsz_count)
plt.bar(reqsz, reqsz_count)
# plt.ylabel('Count')
# plt.xlabel('Request Sizes in Bytes')
plt.show()

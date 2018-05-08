#!/usr/bin/env python3

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from math import pi
import sys

# ------- PART 2: Add plots

# Plot each individual = each line of the data

def percentage(values):
    ''' Transform values in prercentage values. '''
    total = int(values[-1][:-2]) / 100
    print(total)
    print(len(values[:-1]))
    return [int(v[:-2]) / total for v in values[:-1]]


if __name__ == '__main__':

    if len(sys.argv) < 2:
        sys.exit("Error: Need csv file as argument")

    print(sys.argv[1])
    df = pd.read_csv(sys.argv[1])

    # ------- PART 1: Create background

    # number of variable
    categories=list(df)[1:-2]
    print(categories)
    N = len(categories)
    print(N)

    # What will be the angle of each axis in the plot? (we divide the plot / number of variable)
    angles = [n / float(N) * 2 * pi for n in range(N)]
    angles += angles[:1]

    # Initialise the spider plot
    ax = plt.subplot(111, polar=True)

    # If you want the first axis to be on top:
    ax.set_theta_offset(pi / 2)
    ax.set_theta_direction(-1)

    # Draw one axe per variable + add labels labels yet
    plt.xticks(angles[:-1], categories)

    # Draw ylabels
    ax.set_rlabel_position(0)
    plt.yticks([10,20,30,40,50,60,70,80,90,100], ["10","20","30","40","50","60","70","80","90","100"], color="grey", size=7)
    plt.ylim(0,100)

    for idx in df.index:
        values = df.loc[idx].drop('set').values.flatten().tolist()[:-1]
        print(df.loc[idx])
        pvalues = percentage(values)
        str_label = "Set {}".format(idx)
        print(pvalues)
        pvalues += pvalues[:1]
        ax.plot(angles, pvalues, linewidth=1, linestyle='solid', label=str_label)
        ax.fill(angles, pvalues, 'b', alpha=0.1)

    # Add legend
    plt.legend(loc='upper right', bbox_to_anchor=(0.1, 0.1))
    name = "significant_kind.png"
    plt.savefig(name, bbox_inches='tight')

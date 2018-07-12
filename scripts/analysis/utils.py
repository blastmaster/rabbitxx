import matplotlib.pyplot as plt

''' Data '''

def calculate_bandwidth(row):

    raise NotImplementedError

''' Plotting '''

def timelines(y, xstart, xstop, color='b') -> None:
    ''' Plot a horizontal line from xstart to xstop at yaxis postion y. '''
    # print('y: {} xstart: {} xstop: {}'.format(y, xstart, xstop))
    plt.hlines(y, xstart, xstop, color, lw=12)


def gen_set_labels(set_list):
    ''' Generate the labels for diagrams ['Set 1' 'Set 2' ...] '''
    return ["Set {}".format(str(i+1)) for i, _ in enumerate(set_list)]


def byte_to_kilobyte_label(byte_label: str) -> str:
    ''' Helper function to transform labels of bytes into labels of KB '''
    byte_label_value = int(float(byte_label))
    return '{} KB'.format(int(byte_label_value / 1000))

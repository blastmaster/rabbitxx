# Analysis Modules

List of currently available analysis modules. Most of them follow the syntax.
` ./module /path/to/tracefile.otf2 `

| Module | Description |
| ------ | ----------- |
| creates_in_dir | gather concurrent creates within the same directory per cio set |
| global_vs_local | files accessed on local vs file accessed on global parallel file system per cio set |
| io_timespan | durations for each I/O event *not bound on cio sets* |
| open_per_file | Prints how often a file were opened. |
| ops_per_file | Prints for each file which operations how often are executed. |
| print_graph | Print graph as dot file which can be visualized with graphviz. |
| set2csv | Dump I/O events within set as csv, each set results in individual file. |

*TODO* provide more detailed explanation of relevant modules.

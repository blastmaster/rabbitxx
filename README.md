# Rabbitxx

## Description

An I/O pattern analysis tool for parallel applications.
Rabbitxx is a post-mortem analysis tool that identifies parallel I/O operations and investigates their access semantics with regard to POSIX.
Therefore, rabbitxx analyzes an OTF2 trace file of an application run.

## Dependencies

The following libraries are include as submodule.

* [Catch](https://github.com/philsquared/Catch) - Test Framework
* [nitro](https://github.com/tud-zih-energy/nitro) - utility library
* [otf2xx](https://github.com/blastmaster/otf2xx) - Open Trace Format 2 modern C++ wrapper
* [BGL](https://github.com/boostorg/graph) - The Boost graph library
* [Boost filesystem](https://github.com/boostorg/filesystem) - The Boost filesystem library

## Installation

```
git clone --recursive https://github.com/blastmaster/rabbitxx.git
cd rabbitxx; mkdir build; cd build;
cmake ../
make
```

## Run

1. Generate CIO-Sets from a trace file.
2. Perform the analysis modules on the CIO-Sets.

### Generating CIO-Sets from an OTF2-Trace

```
cd build/modules/set2csv
./set2csv /path/to/your/traces.otf2
```

### Run analysis modules

```
cd scripts/
python3 ./run_analysis -c -ov -rmw /path/to/rabbixx-out-dir
```

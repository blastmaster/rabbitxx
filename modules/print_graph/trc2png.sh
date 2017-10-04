#! /bin/bash

set -x

# ./trc2png.sh ~/traces/dios/dios_linktime_traces 
# Takes a directory and creates all png files of the I/O graphs of these traces.

# TODO: create outputs in separate directory

startdir=${1:?"No trace dir given"}

for f in $(find ${startdir} -type f -name traces.otf2 -printf "%h\n"); do
    trcfile="$f/traces.otf2"
    dotfile="${f##*/}.dot"
    outfile="${f##*/}.png"

    ./print-graph $trcfile $dotfile
    if [[ $? -eq 0 ]]; then
        dot -Tpng $dotfile > $outfile
    fi
    echo -e "\n"
done

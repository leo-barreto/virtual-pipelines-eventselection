#!/bin/bash

BASE_DIR=$1

# Compile executable
echo ">>> Compile skimming executable ..."
time g++ -g -std=c++11 -O3 -Wall -Wextra -Wpedantic -o skim skim.cxx $(root-config --cflags --libs)

# Skim samples
while IFS=, read -r SAMPLE XSEC LUMI
do
    echo ">>> Skim sample ${SAMPLE}"
    INPUT=${BASE_DIR}/${SAMPLE}.root
    OUTPUT=${BASE_DIR}/${SAMPLE}Skim.root
    ./skim $INPUT $OUTPUT $XSEC $LUMI
done < samples.csv

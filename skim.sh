#!/bin/bash

BASE_DIR=$1

# Compile executable
echo ">>> Compile skimming executable ..."
time g++-7 -g -std=c++11 -O3 -Wall -Wextra -Wpedantic -o skim skim.cxx $(root-config --cflags --libs)

# Skim samples
while IFS=, read -r SAMPLE XSEC
do
    echo ">>> Skim sample ${SAMPLE}"
    INPUT=${BASE_DIR}/${SAMPLE}.root
    OUTPUT=${BASE_DIR}/${SAMPLE}Skim.root
    LUMI=11467.0 # Integrated luminosity of the unscaled dataset
    SCALE=0.1 # Same fraction as used to down-size the analysis
    ./skim $INPUT $OUTPUT $XSEC $LUMI $SCALE
done < skim.csv

#!/bin/bash

INPUT_DIR=$1
OUTPUT_DIR=$2

# Produce histograms from skimmed samples
while IFS=, read -r SAMPLE PROCESS
do
    INPUT=${INPUT_DIR}/${SAMPLE}Skim.root
    OUTPUT=${OUTPUT_DIR}/histograms_${PROCESS}.root
    python histograms.py $INPUT $PROCESS $OUTPUT
done < histograms.csv

# Merge histograms in a single file
hadd -f ${OUTPUT_DIR}/histograms.root ${OUTPUT_DIR}/histograms_*.root


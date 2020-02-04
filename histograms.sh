#!/bin/bash

BASE_DIR=$1

# Produce histograms from skimmed samples
while IFS=, read -r SAMPLE PROCESS
do
    INPUT=${BASE_DIR}/${SAMPLE}Skim.root
    OUTPUT=${BASE_DIR}/histograms_${PROCESS}.root
    python histograms.py $INPUT $PROCESS $OUTPUT
done < histograms.csv

# Merge histograms in a single file
hadd -f ${BASE_DIR}/histograms.root ${BASE_DIR}/histograms_*.root

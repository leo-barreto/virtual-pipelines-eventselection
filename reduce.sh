#!/bin/bash

OUTPUT_DIR=$1
BASEPATH="root://eospublic.cern.ch//eos/opendata/cms/derived-data/AOD2NanoAODOutreachTool/"

SAMPLES=("GluGluToHToTauTau"
         "VBF_HToTauTau"
         "DYJetsToLL"
         "TTbar"
         "W1JetsToLNu"
         "W2JetsToLNu"
         "W3JetsToLNu"
         "Run2012B_TauPlusX"
         "Run2012C_TauPlusX")

for SAMPLE in ${SAMPLES[@]}
do
    FULLPATH=${BASEPATH}${SAMPLE}.root
    python reduce.py $FULLPATH 0.1 $OUTPUT_DIR
done

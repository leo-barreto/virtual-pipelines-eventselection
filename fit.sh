#!/bin/bash

HISTOGRAMS=$1
OUTPUT=$2

python fit.py $HISTOGRAMS $OUTPUT

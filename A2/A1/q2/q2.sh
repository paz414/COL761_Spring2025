#!/bin/bash
GSPAN=$1
FSG=$2
GASTON=$3
DATASET=$4
OUT_DIR=$5

mkdir -p "$OUT_DIR"

python3 driver.py --gspan "$GSPAN" --fsg "$FSG" --gaston "$GASTON" --dataset "$DATASET" --outdir "$OUT_DIR"
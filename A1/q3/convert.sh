#!/bin/bash
set -euo pipefail

# Usage:
# bash convert.sh <path_graphs> <path_discriminative_subgraphs> <path_features>
GRAPHS="$1"
FEATURES="$2"
OUT_NPY="$3"

python3 convert.py "$GRAPHS" "$FEATURES" "$OUT_NPY"

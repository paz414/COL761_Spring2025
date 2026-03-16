#!/bin/bash
set -euo pipefail

# Usage:
# bash identify.sh <path_graph_dataset> <path_discriminative_subgraphs>
DATASET="$1"
OUT_FEATURES="$2"

python3 identify.py "$DATASET" "$OUT_FEATURES"

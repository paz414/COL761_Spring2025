#!/bin/bash
set -euo pipefail

# Usage:
# bash generate_candidates.sh <path_database_graph_features> <path_query_graph_features> <path_out_file>
DB_NPY="$1"
Q_NPY="$2"
OUT_FILE="$3"

python3 generate_candidates.py "$DB_NPY" "$Q_NPY" "$OUT_FILE"

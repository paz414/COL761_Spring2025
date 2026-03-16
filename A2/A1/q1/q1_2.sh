#!/bin/bash
set -euo pipefail

# Usage:
# bash q1_2.sh <universal_itemset> <num_transactions>
#
# <universal_itemset> : symbolic argument (dataset universe is defined in generate_dt.py)
# <num_transactions>  : number of transactions to generate

UNIVERSE=${1:-""}
NUM_TRANS=${2:-15000}

# Binaries (relative to current directory)
APRIORI_BIN="$(pwd)/apriori/apriori/src/apriori"
FPGROWTH_BIN="$(pwd)/fpgrowth/fpgrowth/src/fpgrowth"

DATASET="generated_transactions.dat"
OUTDIR="synthetic_out"

# Step 1: Generate dataset with the tuned params you specified
python3 generate_dt.py \
  --num_transactions "$NUM_TRANS" \
  --out_file "$DATASET" \
  --cluster_size 80 \
  --p_in 0.40 \
  --num_bridge_items 25 \
  --p_bridge 0.30

# Clean output dir
rm -rf "$OUTDIR"
mkdir -p "$OUTDIR"

# Step 2: Run Apriori + FP-Growth on generated dataset
bash q1_1.sh "$APRIORI_BIN" "$FPGROWTH_BIN" "$DATASET" "$OUTDIR"

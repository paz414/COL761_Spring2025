#!/bin/bash

APRIORI=$1
FPGROWTH=$2
DATASET=$3
OUTDIR=$4

mkdir -p "$OUTDIR"

NTRANS=$(wc -l < "$DATASET")
SUPPORTS=(5 10 25 50 90)



for S in "${SUPPORTS[@]}"; do
    SUPP_COUNT=$(python3 - <<EOF
import math
print(math.ceil($S/100 * $NTRANS))
EOF
)

    echo "Apriori @ ${S}% (minsup=$SUPP_COUNT)"
    /usr/bin/time -v "$APRIORI" -S"$SUPP_COUNT" "$DATASET" \
        > "$OUTDIR/ap${S}" 2> "$OUTDIR/ap${S}_time.txt"

    echo "FP-Growth @ ${S}% (minsup=$SUPP_COUNT)"
    /usr/bin/time -v "$FPGROWTH" -S"$SUPP_COUNT" "$DATASET" \
        > "$OUTDIR/fp${S}" 2> "$OUTDIR/fp${S}_time.txt"
done

# --- Generate times CSV ---
python3 parse_times.py "$OUTDIR" "$OUTDIR/times.csv"

# --- Generate plot ---
python3 plot.py "$OUTDIR/times.csv" "$OUTDIR/plot.png"

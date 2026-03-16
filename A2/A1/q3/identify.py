import sys
import numpy as np
from graph_utils import read_graphs, graph_signature, extract_features

K = 200

def main():
    if len(sys.argv) != 3:
        print("Usage: python identify.py <db_graphs> <out_features>")
        sys.exit(1)

    db_path = sys.argv[1]
    out_path = sys.argv[2]

    graphs = read_graphs(db_path)

    # Deduplicate for feature mining only, preserving first occurrence order
    seen = set()
    unique = []
    for (nl, adj) in graphs:
        sig = graph_signature(nl, adj)
        if sig in seen:
            continue
        seen.add(sig)
        unique.append((nl, adj))

    n = len(unique)
    feat_count = {}

    for (nl, adj) in unique:
        feats = extract_features(nl, adj)
        for f in feats:
            feat_count[f] = feat_count.get(f, 0) + 1

    # Score: p(1-p) where p = freq fraction
    scored = []
    for f, c in feat_count.items():
        p = c / n
        score = p * (1 - p)
        scored.append((score, c, f))

    # Sort: highest discriminativeness, then more support as tie-breaker
    scored.sort(key=lambda x: (x[0], x[1]), reverse=True)

    selected = [f for (_, _, f) in scored[:K]]

    # Write features as plain text, one per line
    # Example line: E 1 3 2   OR   P 1 1 2 2 3  OR  T ...
    with open(out_path, "w") as out:
        for f in selected:
            out.write(" ".join(map(str, f)) + "\n")

    print(f"Unique graphs used for mining: {n}")
    print(f"Selected {len(selected)} features -> {out_path}")

if __name__ == "__main__":
    main()

import sys
import numpy as np
from graph_utils import read_graphs, extract_features

def load_features(path):
    feats = []
    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            # First token is feature type 'E'/'P'/'T'
            t = parts[0]
            rest = list(map(int, parts[1:]))
            feats.append(tuple([t] + rest))
    return feats

def main():
    if len(sys.argv) != 4:
        print("Usage: python convert.py <graphs> <features_txt> <out_npy>")
        sys.exit(1)

    graphs_path = sys.argv[1]
    feats_path = sys.argv[2]
    out_npy = sys.argv[3]

    features = load_features(feats_path)
    k = len(features)

    graphs = read_graphs(graphs_path)
    m = len(graphs)

    X = np.zeros((m, k), dtype=np.uint8)

    # Precompute for each graph: its feature set
    for i, (nl, adj) in enumerate(graphs):
        gfeats = extract_features(nl, adj)
        # Fill vector
        for j, f in enumerate(features):
            if f in gfeats:
                X[i, j] = 1

    np.save(out_npy, X)
    print(f"Saved features: shape={X.shape} -> {out_npy}")

if __name__ == "__main__":
    main()

import hashlib
from collections import defaultdict


def read_graphs(path):
    """
    Reads graphs in the assignment format:
      #                (new graph marker)
      v <id> <label>
      e <u> <v> <edge_label>
    Returns a list of (node_labels, adj) pairs.
      node_labels: dict[int -> int]
      adj: defaultdict(list) where adj[u] = list of (v, edge_label)
    """
    graphs = []
    node_labels = None
    adj = None

    def flush():
        nonlocal node_labels, adj
        if node_labels is None:
            return
        graphs.append((node_labels, adj))
        node_labels = None
        adj = None

    with open(path, "r") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue

            if line.startswith("#"):
                flush()
                node_labels = {}
                adj = defaultdict(list)
                continue

            parts = line.split()
            if parts[0] == "v":
                vid = int(parts[1])
                vlab = int(parts[2])
                node_labels[vid] = vlab
            elif parts[0] == "e":
                u = int(parts[1])
                v = int(parts[2])
                el = int(parts[3])
                # Treat as undirected for feature extraction
                adj[u].append((v, el))
                adj[v].append((u, el))

    flush()
    return graphs


def graph_signature(node_labels, adj):
    """
    Cheap canonical-ish signature for dedup:
    - multiset of node labels
    - multiset of edge triples (min(uLabel,vLabel), eLabel, max(uLabel,vLabel))
    """
    nl = sorted(node_labels.values())

    edges = []
    seen = set()
    for u, nbrs in adj.items():
        for v, el in nbrs:
            a, b = (u, v) if u < v else (v, u)
            key = (a, b, el)
            if key in seen:
                continue
            seen.add(key)
            lu, lv = node_labels[u], node_labels[v]
            x, y = (lu, lv) if lu <= lv else (lv, lu)
            edges.append((x, el, y))

    edges.sort()
    data = (tuple(nl), tuple(edges))
    return hashlib.sha1(repr(data).encode("utf-8")).hexdigest()


def extract_features(node_labels, adj):
    """
    Extract small monotone graph fragments (features) present in the graph.
    Features:
      - Edge:      ("E", a, el, b) where a<=b are node labels
      - Path-2:    ("P", a, e1, m, e2, c)  (m is center node label), canonicalized by ends
      - Path-3:    ("Q", la, e_ab, lb, e_bc, lc, e_cd, ld), canonicalized by direction
      - Triangle:  ("T", l1,l2,l3, e12,e23,e31) canonicalized by min over permutations

    Returns: set of feature tuples.
    """
    feats = set()

    # ---- Edge features ----
    edge_seen = set()
    for u, nbrs in adj.items():
        for v, el in nbrs:
            a, b = (u, v) if u < v else (v, u)
            key = (a, b, el)
            if key in edge_seen:
                continue
            edge_seen.add(key)
            lu, lv = node_labels[u], node_labels[v]
            x, y = (lu, lv) if lu <= lv else (lv, lu)
            feats.add(("E", x, el, y))

    # ---- Path length-2 features ----
    # For each center node m, take unordered pairs of neighbors (u, v)
    for m, nbrs in adj.items():
        lm = node_labels[m]
        L = len(nbrs)
        for i in range(L):
            u, e1 = nbrs[i]
            lu = node_labels[u]
            for j in range(i + 1, L):
                v, e2 = nbrs[j]
                lv = node_labels[v]

                left = (lu, e1)
                right = (lv, e2)
                if left <= right:
                    a, ea = left
                    c, ec = right
                else:
                    a, ea = right
                    c, ec = left

                feats.add(("P", a, ea, lm, ec, c))

    # ---- Path length-3 features ----
    # Enumerate a-b-c-d where (b,c) is the middle edge
    nbrs_map = {u: list(adj[u]) for u in adj}
    for b, b_nbrs in nbrs_map.items():
        lb = node_labels[b]
        for c, e_bc in b_nbrs:
            lc = node_labels[c]

            # neighbors a of b excluding c
            for a, e_ab in b_nbrs:
                if a == c:
                    continue
                la = node_labels[a]

                # neighbors d of c excluding b
                for d, e_cd in nbrs_map.get(c, []):
                    if d == b:
                        continue
                    ld = node_labels[d]

                    fwd = ("Q", la, e_ab, lb, e_bc, lc, e_cd, ld)
                    rev = ("Q", ld, e_cd, lc, e_bc, lb, e_ab, la)
                    feats.add(fwd if fwd <= rev else rev)

    # ---- Triangle features ----
    # Build neighbor dict for quick edge-label lookup
    nbr_label = {}
    for u, nbrs in adj.items():
        d = {}
        for v, el in nbrs:
            d[v] = el
        nbr_label[u] = d

    nodes = sorted(node_labels.keys())
    import itertools

    for i in range(len(nodes)):
        a = nodes[i]
        for b, eab in adj[a]:
            if b <= a:
                continue
            na = nbr_label[a]
            nb = nbr_label[b]
            common = set(na.keys()).intersection(nb.keys())
            for c in common:
                if c <= b:
                    continue

                eac = na[c]
                ebc = nb[c]
                la, lb, lc = node_labels[a], node_labels[b], node_labels[c]

                # Canonicalize by checking all permutations of (a,b,c) labels
                # while mapping edge labels correctly
                verts = [(la, "a"), (lb, "b"), (lc, "c")]
                edge_label = {
                    ("a", "b"): eab, ("b", "a"): eab,
                    ("a", "c"): eac, ("c", "a"): eac,
                    ("b", "c"): ebc, ("c", "b"): ebc,
                }

                best = None
                for perm in itertools.permutations(verts, 3):
                    (l1, t1), (l2, t2), (l3, t3) = perm
                    rep = ("T", l1, l2, l3,
                           edge_label[(t1, t2)],
                           edge_label[(t2, t3)],
                           edge_label[(t3, t1)])
                    if best is None or rep < best:
                        best = rep

                feats.add(best)

    return feats

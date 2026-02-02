import sys
import numpy as np

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 generate_candidates.py <db.npy> <q.npy> <out_candidates.dat>")
        sys.exit(1)

    db_path = sys.argv[1]
    q_path = sys.argv[2]
    out_path = sys.argv[3]

    DB = np.load(db_path)  # (N, K) uint8
    Q  = np.load(q_path)   # (M, K) uint8

    N, K = DB.shape
    M, K2 = Q.shape
    assert K == K2

    # Support of each feature in the database (how common it is)
    support = DB.sum(axis=0)  # shape (K,)

    # Tuning knobs (keep small & simple)
    ALPHA = 0.5   # enforce ~50% of query's active features (rarest ones)
    TMAX  = 6     # never require more than this many features

    with open(out_path, "w") as out:
        for qi in range(M):
            vq = Q[qi]
            active = np.flatnonzero(vq == 1)

            # If no features in query: cannot prune
            if active.size == 0:
                candidates = list(range(1, N + 1))
            else:
                # Pick rarest query-features first
                active_sorted = active[np.argsort(support[active])]

                # Start with a moderate number of enforced features
                t0 = int(np.floor(ALPHA * active_sorted.size))
                t0 = max(1, t0)
                t0 = min(t0, TMAX, active_sorted.size)

                candidates_idx = None

                # Backoff loop: if too strict -> reduce t until non-empty
                for t in range(t0, 0, -1):
                    must = active_sorted[:t]
                    ok = (DB[:, must] == 1).all(axis=1)
                    idx = np.nonzero(ok)[0]
                    if idx.size > 0:
                        candidates_idx = idx
                        break

                # If still empty, fallback to all graphs (never output empty)
                if candidates_idx is None:
                    candidates = list(range(1, N + 1))
                else:
                    candidates = (candidates_idx + 1).tolist()  # 1-indexed

            out.write(f"q # {qi+1}\n")
            out.write("c # " + " ".join(map(str, candidates)) + "\n")

    print(f"Wrote candidates -> {out_path}")

if __name__ == "__main__":
    main()

import random

def generate_dataset(
    core_items,
    medium_items,
    rare_items,
    num_transactions,
    out_file
):
    seen = set()

    with open(out_file, "w") as f:
        while len(seen) < num_transactions:
            txn = set()

            # core items (almost always)
            if random.random() < 0.9:
                txn.update(core_items)

            # medium-frequency items
            k_med = random.randint(3, 8)
            txn.update(random.sample(medium_items, k_med))

            # rare items
            if random.random() < 0.3:
                k_rare = random.randint(1, 2)
                txn.update(random.sample(rare_items, k_rare))

            txn = tuple(sorted(txn))
            if txn in seen:
                continue

            seen.add(txn)
            f.write(" ".join(txn) + "\n")
core_items   = [f"C{i}" for i in range(6)]
medium_items = [f"M{i}" for i in range(80)]
rare_items   = [f"R{i}" for i in range(300)]
generate_dataset(core_items, medium_items, rare_items,
                 num_transactions=15000,
                 out_file="generated_transactions.dat")

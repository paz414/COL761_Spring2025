import random

def generate_dataset(num_transactions, out_file):
    seen = set()
    attempts = 0
    max_attempts = num_transactions * 20

    # Medium-frequency pool (Apriori poison)
    poison_items = [f"P{i}" for i in range(60)]

    # Rare noise
    rare_items = [f"R{i}" for i in range(200)]

    with open(out_file, "w") as f:
        while len(seen) < num_transactions and attempts < max_attempts:
            attempts += 1
            txn = set()

            # 70% transactions are dense and overlapping
            if random.random() < 0.7:
                k = random.randint(15, 25)
                txn.update(random.sample(poison_items, k))
            else:
                k = random.randint(5, 10)
                txn.update(random.sample(poison_items, k))

            # add noise
            if random.random() < 0.3:
                txn.update(random.sample(rare_items, 1))

            txn_tuple = tuple(sorted(txn))
            if txn_tuple in seen:
                continue

            seen.add(txn_tuple)
            f.write(" ".join(txn_tuple) + "\n")

    print(f"Generated {len(seen)} transactions")

# ---- RUN ----
generate_dataset(
    num_transactions=15000,
    out_file="generated_transactions.dat"
)

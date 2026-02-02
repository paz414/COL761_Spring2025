import re
import sys

def to_seconds(t):
    parts = t.split(":")
    if len(parts) == 3:      # h:mm:ss
        h, m, s = parts
        return int(h)*3600 + int(m)*60 + float(s)
    elif len(parts) == 2:    # m:ss
        m, s = parts
        return int(m)*60 + float(s)
    else:                    # seconds
        return float(parts[0])

supports = [5,10,25,50,90]
ap_times = []
fp_times = []

out_dir = sys.argv[1] if len(sys.argv) > 1 else "full_out"
csv_file = sys.argv[2] if len(sys.argv) > 2 else "times.csv"

for s in supports:
    with open(f"{out_dir}/ap{s}_time.txt") as f:
        match = re.search(r"Elapsed.*:\s*([0-9:.]+)", f.read())
        ap_times.append(to_seconds(match.group(1)))

    with open(f"{out_dir}/fp{s}_time.txt") as f:
        match = re.search(r"Elapsed.*:\s*([0-9:.]+)", f.read())
        fp_times.append(to_seconds(match.group(1)))

print("Output directory:", out_dir)
print("Apriori (s):", ap_times)
print("FP-Growth (s):", fp_times)

with open(csv_file, "w") as f:
    f.write("support,apriori,fpgrowth\n")
    for s, a, fp in zip(supports, ap_times, fp_times):
        f.write(f"{s},{a},{fp}\n")

print(f"Times written to {csv_file}")

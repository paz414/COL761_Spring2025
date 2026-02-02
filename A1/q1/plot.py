import matplotlib.pyplot as plt
import csv
import sys

# Usage: python3 plot.py <csv_file>
csv_file = sys.argv[1] if len(sys.argv) > 1 else "times.csv"

supports = []
apriori = []
fpgrowth = []

with open(csv_file, "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        supports.append(int(row["support"]))
        apriori.append(float(row["apriori"]))
        fpgrowth.append(float(row["fpgrowth"]))

plt.plot(supports, apriori, marker='o', label='Apriori')
plt.plot(supports, fpgrowth, marker='o', label='FP-Growth')

plt.xlabel("Minimum Support (%)")
plt.ylabel("Runtime (seconds)")
plt.title(f"Runtime Comparison (from {csv_file})")
plt.legend()
plt.grid(True)

out_png = csv_file.replace(".csv", ".png")
plt.savefig(out_png)
plt.show()

print(f"Plot saved as {out_png}")


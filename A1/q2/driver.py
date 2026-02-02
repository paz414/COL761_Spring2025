import sys
import os
import subprocess
import time
import matplotlib.pyplot as plt
from math import ceil
from convert_data import convert_data

def run_experiments(gspan_exe, fsg_exe, gaston_exe, dataset_path, out_dir):
    # Convert Data
    convert_data(dataset_path, out_dir)
    
    gspan_input = os.path.join(out_dir, "input_gspan.txt")
    fsg_input = os.path.join(out_dir, "input_fsg.txt")
    gaston_input = os.path.join(out_dir, "input_gaston.txt")

    # Get total graphs
    total_graphs = 0
    with open(gspan_input, 'r') as f:
        for line in f:
            if line.startswith("t #") and not line.strip().startswith("t # -1"): 
                total_graphs += 1

    # Thresholds
    thresholds = [5, 10, 25, 50, 95]
    runtimes = {'gSpan': [], 'FSG': [], 'Gaston': []}

    print(f"Total Graphs: {total_graphs}")

    for pct in thresholds:
        print(f"\n--- Running Support {pct}% ---")
        
        sup_ratio = pct / 100.0         
        sup_count = ceil(sup_ratio * total_graphs) 
        sup_pct = pct                   

        # Run gSpan
        out_file = os.path.join(out_dir, f"gspan{pct}")
        
        cmd = [gspan_exe, "-f", gspan_input, "-s", str(sup_ratio), "-o"]
        
        print(f"Running: {' '.join(cmd)}")
        start = time.time()
        
        # capture stdout to file
        with open(out_file, "w") as outfile:
            subprocess.run(cmd, stdout=outfile, stderr=subprocess.PIPE)
            
        runtimes['gSpan'].append(time.time() - start)
        print("gSpan finished.")

        # Run FSG
        out_file = os.path.join(out_dir, f"fsg{pct}")
        cmd_str = f"{fsg_exe} -s {sup_pct} {fsg_input} > {out_file}"
        
        print(f"Running: {cmd_str}")
        start = time.time()
        subprocess.run(cmd_str, shell=True)
        runtimes['FSG'].append(time.time() - start)
        print("FSG finished.")

        # Run Gaston
        out_file = os.path.join(out_dir, f"gaston{pct}")
        cmd_str = f"{gaston_exe} {sup_count} {gaston_input} > {out_file}"
        
        print(f"Running: {cmd_str}")
        start = time.time()
        subprocess.run(cmd_str, shell=True)
        runtimes['Gaston'].append(time.time() - start)
        print("Gaston finished.")

    # Plotting
    plt.figure()
    plt.plot(thresholds, runtimes['gSpan'], marker='o', label='gSpan')
    plt.plot(thresholds, runtimes['FSG'], marker='s', label='FSG')
    plt.plot(thresholds, runtimes['Gaston'], marker='^', label='Gaston')
    plt.xlabel('Minimum Support (%)')
    plt.ylabel('Time (s)')
    plt.title('Algorithm Runtime Comparison')
    plt.legend()
    plt.grid(True)
    plt.savefig(os.path.join(out_dir, "plot.png"))
    print("Plot saved.")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--gspan", required=True)
    parser.add_argument("--fsg", required=True)
    parser.add_argument("--gaston", required=True)
    parser.add_argument("--dataset", required=True)
    parser.add_argument("--outdir", required=True)
    args = parser.parse_args()
    
    run_experiments(args.gspan, args.fsg, args.gaston, args.dataset, args.outdir)
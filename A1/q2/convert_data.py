import sys
import os

def convert_data(input_path, output_dir):
    """
    Reads the Yeast dataset, maps string labels to integers, 
    and creates input files for gSpan, FSG, and Gaston.
    """
    gspan_file = os.path.join(output_dir, "input_gspan.txt")
    fsg_file = os.path.join(output_dir, "input_fsg.txt")
    gaston_file = os.path.join(output_dir, "input_gaston.txt")
    
    print(f"Reading dataset: {input_path}")
    
    with open(input_path, 'r') as f:
        lines = [line.strip() for line in f if line.strip()]

    graphs = []
    current_graph = None
    
    # Mappings for labels
    node_label_map = {}
    edge_label_map = {}
    
    def get_node_id(label):
        if label not in node_label_map:
            node_label_map[label] = len(node_label_map)
        return node_label_map[label]

    def get_edge_id(label):
        if label not in edge_label_map:
            edge_label_map[label] = len(edge_label_map)
        return edge_label_map[label]

    i = 0
    while i < len(lines):
        line = lines[i]
        if line.startswith('#'):
            if current_graph:
                graphs.append(current_graph)
            
            graph_id = line.replace('#', '').strip()
            current_graph = {'id': graph_id, 'nodes': [], 'edges': []}
            i += 1
            
            # Parse Nodes
            if i < len(lines):
                num_nodes = int(lines[i])
                i += 1
                for _ in range(num_nodes):
                    # MAP LABEL TO INTEGER
                    original_label = lines[i]
                    mapped_label = get_node_id(original_label)
                    current_graph['nodes'].append(mapped_label)
                    i += 1
            
            # Parse Edges
            if i < len(lines):
                num_edges = int(lines[i])
                i += 1
                for _ in range(num_edges):
                    parts = lines[i].replace(',', ' ').split()
                    src, dst = parts[0], parts[1]
                    # MAP THE LABEL TO INTEGER
                    original_label = parts[2]
                    mapped_label = get_edge_id(original_label)
                    current_graph['edges'].append((src, dst, mapped_label))
                    i += 1
        else:
            i += 1
            
    if current_graph:
        graphs.append(current_graph)

    print(f"Parsed {len(graphs)} graphs.")
    print(f"Unique Node Labels: {len(node_label_map)}")
    print(f"Unique Edge Labels: {len(edge_label_map)}")

    # Write gSpan
    with open(gspan_file, 'w') as f:
        for idx, g in enumerate(graphs):
            f.write(f"t # {idx}\n")
            for node_idx, label in enumerate(g['nodes']):
                f.write(f"v {node_idx} {label}\n")
            for src, dst, label in g['edges']:
                f.write(f"e {src} {dst} {label}\n")
        f.write("t # -1\n") 

    # Write Gaston
    with open(gaston_file, 'w') as f:
        for idx, g in enumerate(graphs):
            f.write(f"t # {idx}\n")
            for node_idx, label in enumerate(g['nodes']):
                f.write(f"v {node_idx} {label}\n")
            for src, dst, label in g['edges']:
                f.write(f"e {src} {dst} {label}\n")
        # No EOF marker

    # Write FSG
    with open(fsg_file, 'w') as f:
        for idx, g in enumerate(graphs):
            f.write(f"t # {idx}\n")
            for node_idx, label in enumerate(g['nodes']):
                f.write(f"v {node_idx} {label}\n")
            for src, dst, label in g['edges']:
                f.write(f"u {src} {dst} {label}\n")

    print("Conversion complete.")

if __name__ == "__main__":
    convert_data(sys.argv[1], sys.argv[2])
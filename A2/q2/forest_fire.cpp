#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include <random>
#include <numeric>
#include <cmath>
#include <cassert>
#include <set>

using namespace std;

// ─────────────────────────────────────────────
// 1. DATA STRUCTURES & PARSING
// ─────────────────────────────────────────────

struct Edge {
    int u, v;
    double p;
    int idx;
};

typedef uint64_t EdgeKey;
inline EdgeKey make_key(int u, int v) {
    return ((uint64_t)(unsigned int)u << 32) | (unsigned int)v;
}
inline pair<int,int> from_key(EdgeKey k) {
    return {(int)(k >> 32), (int)(k & 0xFFFFFFFF)};
}

int max_node_id = 0;

pair<int, vector<Edge>> load_graph(const string& graph_path) {
    ifstream fin(graph_path);
    if (!fin) { cerr << "Error opening graph file.\n"; exit(1); }
    
    vector<Edge> edges;
    int u, v;
    double p;
    int max_n = 0;
    int idx = 0;
    string line;
    
    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (sscanf(line.c_str(), "%d %d %lf", &u, &v, &p) == 3) {
            edges.push_back({u, v, p, idx++});
            if (u > max_n) max_n = u;
            if (v > max_n) max_n = v;
        }
    }
    
    max_node_id = max_n;
    return {max_n, edges};
}

vector<int> load_seeds(const string& seed_path) {
    ifstream fin(seed_path);
    if (!fin) { cerr << "Error opening seed file.\n"; exit(1); }
    vector<int> seeds;
    int s;
    while (fin >> s) seeds.push_back(s);
    return seeds;
}

// ─────────────────────────────────────────────
// 2. ADJACENCY LIST (flat, cache-friendly)
// ─────────────────────────────────────────────

struct AdjList {
    vector<int> start;
    vector<int> target;
    vector<double> prob;
    int n;
    
    void build(int max_id, const vector<Edge>& edges, const unordered_set<EdgeKey>& blocked) {
        n = max_id + 1;
        vector<int> count(n, 0);
        for (const auto& e : edges)
            if (blocked.find(make_key(e.u, e.v)) == blocked.end())
                count[e.u]++;
        
        start.resize(n + 1);
        start[0] = 0;
        for (int i = 0; i < n; i++)
            start[i + 1] = start[i] + count[i];
        
        int total = start[n];
        target.resize(total);
        prob.resize(total);
        fill(count.begin(), count.end(), 0);
        
        for (const auto& e : edges) {
            if (blocked.find(make_key(e.u, e.v)) == blocked.end()) {
                int pos = start[e.u] + count[e.u];
                target[pos] = e.v;
                prob[pos] = e.p;
                count[e.u]++;
            }
        }
    }
    
    int out_degree(int u) const { return start[u + 1] - start[u]; }
};

// BFS distances from seeds
vector<int> bfs_dist(const AdjList& adj, const vector<int>& seeds, int hops) {
    vector<int> dist(adj.n, -1);
    queue<int> q;
    for (int s : seeds) {
        if (s < adj.n) { dist[s] = 0; q.push(s); }
    }
    while (!q.empty()) {
        int u = q.front(); q.pop();
        if (hops > 0 && dist[u] >= hops) continue;
        for (int i = adj.start[u]; i < adj.start[u + 1]; i++) {
            int v = adj.target[i];
            if (dist[v] == -1) { dist[v] = dist[u] + 1; q.push(v); }
        }
    }
    return dist;
}

// ─────────────────────────────────────────────
// 3. SIMULATION
// ─────────────────────────────────────────────

double simulate(const AdjList& adj, const vector<int>& seeds,
                const vector<int>& dist, bool use_hops, int hops,
                int n_sims, mt19937& rng) {
    double total = 0;
    vector<bool> burned(adj.n, false);
    vector<int> to_clear, frontier, next_frontier;
    uniform_real_distribution<double> unif(0.0, 1.0);
    
    for (int sim = 0; sim < n_sims; sim++) {
        int count = 0;
        for (int s : seeds) {
            if (s < adj.n && !burned[s]) {
                burned[s] = true;
                to_clear.push_back(s);
                frontier.push_back(s);
                count++;
            }
        }
        
        while (!frontier.empty()) {
            for (int u : frontier) {
                for (int i = adj.start[u]; i < adj.start[u + 1]; i++) {
                    int v = adj.target[i];
                    if (!burned[v]) {
                        if (use_hops && (dist[v] == -1 || dist[v] > hops)) continue;
                        if (unif(rng) < adj.prob[i]) {
                            burned[v] = true;
                            to_clear.push_back(v);
                            next_frontier.push_back(v);
                            count++;
                        }
                    }
                }
            }
            frontier.swap(next_frontier);
            next_frontier.clear();
        }
        
        total += count;
        for (int v : to_clear) burned[v] = false;
        to_clear.clear();
        frontier.clear();
    }
    return total / n_sims;
}

// Consistent-seed simulation (for fair CELF comparisons)
double simulate_consistent(const AdjList& adj, const vector<int>& seeds,
                           const vector<int>& dist, bool use_hops, int hops,
                           int n_sims) {
    mt19937 rng(42); // Always same seed for consistent comparison
    return simulate(adj, seeds, dist, use_hops, hops, n_sims, rng);
}

// ─────────────────────────────────────────────
// 4. TIMER
// ─────────────────────────────────────────────

chrono::steady_clock::time_point t0;
double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - t0).count();
}

// ─────────────────────────────────────────────
// 5. WRITE OUTPUT
// ─────────────────────────────────────────────

void write_output(const string& path, const vector<pair<int,int>>& blocked_list) {
    ofstream fout(path);
    for (auto& [u, v] : blocked_list)
        fout << u << " " << v << "\n";
}

// ─────────────────────────────────────────────
// 6. HEURISTIC: Score edges by expected impact
// ─────────────────────────────────────────────

struct CandEdge {
    double score;
    int eidx;
    int u, v;
    double p;
};

vector<CandEdge> rank_candidates(const vector<Edge>& edges, const vector<int>& seeds,
                                  int hops, const unordered_set<EdgeKey>& blocked,
                                  const AdjList& adj) {
    auto dist = bfs_dist(adj, seeds, hops);
    unordered_set<int> seed_set(seeds.begin(), seeds.end());
    
    // Compute reach probability estimate using BFS with probability propagation
    // For each node, estimate P(fire reaches this node)
    vector<double> reach_prob(adj.n, 0.0);
    for (int s : seeds) if (s < adj.n) reach_prob[s] = 1.0;
    
    // BFS layer by layer, propagate probabilities
    vector<int> layer;
    vector<int> next_layer;
    vector<bool> visited(adj.n, false);
    for (int s : seeds) { 
        if (s < adj.n) { layer.push_back(s); visited[s] = true; }
    }
    
    int depth = 0;
    while (!layer.empty()) {
        depth++;
        if (hops > 0 && depth > hops) break;
        for (int u : layer) {
            for (int i = adj.start[u]; i < adj.start[u + 1]; i++) {
                int v = adj.target[i];
                double p = adj.prob[i];
                // P(v reached) ≈ 1 - (1 - P(u reached)*p) * (1 - existing)
                // Simplified: add contribution
                double contrib = reach_prob[u] * p;
                reach_prob[v] = 1.0 - (1.0 - reach_prob[v]) * (1.0 - contrib);
                if (!visited[v]) {
                    visited[v] = true;
                    next_layer.push_back(v);
                }
            }
        }
        layer.swap(next_layer);
        next_layer.clear();
    }
    
    // For each edge, estimate impact of blocking it:
    // impact ≈ P(u reached) * p(u,v) * expected_cascade_from_v
    // We approximate expected_cascade_from_v using subtree BFS count (limited depth)
    vector<CandEdge> candidates;
    for (const auto& e : edges) {
        if (blocked.find(make_key(e.u, e.v)) != blocked.end()) continue;
        if (dist[e.u] == -1) continue;
        if (hops > 0 && dist[e.v] != -1 && dist[e.v] > hops) continue;
        if (hops > 0 && dist[e.v] == -1) continue;
        
        double pr_u = reach_prob[e.u];
        
        // Downstream estimate: count nodes reachable from v (limited BFS)
        // But this is expensive for all edges; use out_degree as fast proxy
        double downstream = 1.0 + adj.out_degree(e.v);
        
        // Bonus for seed edges (distance 0): they cut off ALL downstream
        double seed_bonus = (dist[e.u] == 0) ? 5.0 : 1.0;
        
        // Score: higher = more valuable to block
        double score = pr_u * e.p * downstream * seed_bonus;
        
        candidates.push_back({score, e.idx, e.u, e.v, e.p});
    }
    
    sort(candidates.begin(), candidates.end(), [](const CandEdge& a, const CandEdge& b) {
        return a.score > b.score;
    });
    
    return candidates;
}

// ─────────────────────────────────────────────
// 7. MAIN ALGORITHM
// ─────────────────────────────────────────────

struct HeapNode {
    double gain;
    int cand_idx;
    int last_iter;
    bool operator<(const HeapNode& other) const { return gain < other.gain; }
};

vector<pair<int,int>> solve(const vector<Edge>& edges, const vector<int>& seeds,
                             int k, int n_sims_param, int hops,
                             const string& output_path, double time_limit) {
    bool use_hops = (hops > 0);
    unordered_set<EdgeKey> blocked_set;
    vector<pair<int,int>> blocked_list;
    
    // Build initial adj
    AdjList adj;
    adj.build(max_node_id, edges, blocked_set);
    
    // ====== PHASE 0: Quick seed-edge strategy ======
    // Compute initial greedy: block highest-probability edges from seeds first
    auto dist0 = bfs_dist(adj, seeds, hops);
    
    // Collect all edges from seed nodes, sorted by probability (descending)
    vector<pair<double, int>> seed_edges; // (probability, edge_index)
    unordered_set<int> seed_set(seeds.begin(), seeds.end());
    for (const auto& e : edges) {
        if (seed_set.count(e.u)) {
            if (hops > 0 && dist0[e.v] != -1 && dist0[e.v] <= hops) {
                seed_edges.push_back({e.p, e.idx});
            } else if (!use_hops) {
                seed_edges.push_back({e.p, e.idx});
            }
        }
    }
    sort(seed_edges.begin(), seed_edges.end(), [](auto& a, auto& b) {
        return a.first > b.first;
    });
    
    cerr << "Seed edges available: " << seed_edges.size() << "\n";
    
    // Quick initial solution: block top-k seed edges by probability
    vector<pair<int,int>> initial_solution;
    unordered_set<EdgeKey> initial_blocked;
    for (int i = 0; i < min(k, (int)seed_edges.size()); i++) {
        int eidx = seed_edges[i].second;
        initial_blocked.insert(make_key(edges[eidx].u, edges[eidx].v));
        initial_solution.push_back({edges[eidx].u, edges[eidx].v});
    }
    
    // If seed edges don't fill budget, add best non-seed edges by heuristic
    if ((int)initial_solution.size() < k) {
        auto cands = rank_candidates(edges, seeds, hops, initial_blocked, adj);
        for (auto& c : cands) {
            if ((int)initial_solution.size() >= k) break;
            EdgeKey key = make_key(c.u, c.v);
            if (initial_blocked.find(key) == initial_blocked.end()) {
                initial_blocked.insert(key);
                initial_solution.push_back({c.u, c.v});
            }
        }
    }
    
    // Evaluate initial solution
    AdjList adj_init;
    adj_init.build(max_node_id, edges, initial_blocked);
    auto dist_init = bfs_dist(adj_init, seeds, hops);
    double spread_init = simulate_consistent(adj_init, seeds, dist_init, use_hops, hops, 
                                              max(200, n_sims_param * 4));
    
    // Save as best so far
    blocked_list = initial_solution;
    blocked_set = initial_blocked;
    write_output(output_path, blocked_list);
    
    double baseline_spread = simulate_consistent(adj, seeds, 
                                                  bfs_dist(adj, seeds, hops), 
                                                  use_hops, hops, 
                                                  max(200, n_sims_param * 4));
    
    cerr << "Phase 0: Seed-edge strategy, spread=" << spread_init 
         << " (baseline=" << baseline_spread << ")"
         << ", rho=" << (baseline_spread - spread_init)/baseline_spread 
         << ", t=" << elapsed() << "s\n";
    
    double best_spread = spread_init;
    
    // ====== PHASE 1: CELF refinement ======
    // Now try CELF greedy from scratch to see if it finds a better solution
    double celf_time_budget = time_limit * 0.45;
    
    // Calibrate simulation speed
    double t_cal = elapsed();
    {
        mt19937 rng_cal(99);
        simulate(adj, seeds, bfs_dist(adj, seeds, hops), use_hops, hops, 50, rng_cal);
    }
    double time_per_sim = (elapsed() - t_cal) / 50.0;
    cerr << "Time per sim: " << time_per_sim * 1000 << "ms\n";
    
    // Determine sim counts
    int sims_screen, sims_confirm;
    {
        auto full_cands = rank_candidates(edges, seeds, hops, {}, adj);
        int n_cands = min((int)full_cands.size(), max(2000, k * 30));
        double screen_budget = celf_time_budget * 0.4;
        sims_screen = max(30, min(500, (int)(screen_budget / (n_cands * time_per_sim))));
        sims_confirm = max(sims_screen * 2, min(1000, sims_screen * 5));
    }
    cerr << "CELF sims: screen=" << sims_screen << ", confirm=" << sims_confirm << "\n";
    
    // Run CELF from scratch
    unordered_set<EdgeKey> celf_blocked;
    vector<pair<int,int>> celf_list;
    
    auto celf_cands = rank_candidates(edges, seeds, hops, celf_blocked, adj);
    int n_celf_cands = min((int)celf_cands.size(), max(2000, k * 30));
    celf_cands.resize(n_celf_cands);
    
    cerr << "CELF candidates: " << n_celf_cands << "\n";
    
    // Build baseline for CELF
    AdjList celf_adj;
    celf_adj.build(max_node_id, edges, celf_blocked);
    auto celf_dist = bfs_dist(celf_adj, seeds, hops);
    double celf_baseline = simulate_consistent(celf_adj, seeds, celf_dist, use_hops, hops, sims_confirm);
    
    // Initial evaluation of all candidates
    priority_queue<HeapNode> heap;
    
    for (int i = 0; i < n_celf_cands; i++) {
        if (elapsed() > celf_time_budget * 0.5) {
            cerr << "CELF init cutoff at " << i << "/" << n_celf_cands << "\n";
            break;
        }
        
        auto& c = celf_cands[i];
        EdgeKey trial_key = make_key(c.u, c.v);
        unordered_set<EdgeKey> trial_blocked = celf_blocked;
        trial_blocked.insert(trial_key);
        
        AdjList trial_adj;
        trial_adj.build(max_node_id, edges, trial_blocked);
        auto trial_dist = bfs_dist(trial_adj, seeds, hops);
        double spread = simulate_consistent(trial_adj, seeds, trial_dist, use_hops, hops, sims_screen);
        double gain = celf_baseline - spread;
        heap.push({gain, i, 0});
    }
    
    cerr << "CELF init done (" << heap.size() << " candidates), t=" << elapsed() << "s\n";
    
    // CELF greedy iterations
    double celf_deadline = celf_time_budget + elapsed() * 0.1; // give extra
    
    for (int iter = 0; iter < k; iter++) {
        if (elapsed() > celf_deadline) {
            cerr << "CELF deadline at iter " << iter << "\n";
            break;
        }
        if (heap.empty()) break;
        
        bool found = false;
        while (!heap.empty()) {
            if (elapsed() > celf_deadline + 30) {
                // Emergency pick
                auto top = heap.top(); heap.pop();
                auto& c = celf_cands[top.cand_idx];
                EdgeKey key = make_key(c.u, c.v);
                if (celf_blocked.find(key) != celf_blocked.end()) continue;
                celf_blocked.insert(key);
                celf_list.push_back({c.u, c.v});
                found = true;
                cerr << "  CELF [" << iter+1 << "/" << k << "] EMERGENCY (" << c.u << "," << c.v << ")\n";
                break;
            }
            
            auto top = heap.top(); heap.pop();
            auto& c = celf_cands[top.cand_idx];
            EdgeKey key = make_key(c.u, c.v);
            if (celf_blocked.find(key) != celf_blocked.end()) continue;
            
            if (top.last_iter == iter) {
                // Accept
                celf_blocked.insert(key);
                celf_list.push_back({c.u, c.v});
                found = true;
                
                // Update baseline
                celf_adj.build(max_node_id, edges, celf_blocked);
                celf_dist = bfs_dist(celf_adj, seeds, hops);
                celf_baseline = simulate_consistent(celf_adj, seeds, celf_dist, use_hops, hops, sims_confirm);
                
                cerr << "  CELF [" << iter+1 << "/" << k << "] (" << c.u << "," << c.v 
                     << "), spread=" << celf_baseline << ", t=" << elapsed() << "s\n";
                break;
            } else {
                // Re-evaluate
                unordered_set<EdgeKey> trial_blocked = celf_blocked;
                trial_blocked.insert(key);
                AdjList trial_adj;
                trial_adj.build(max_node_id, edges, trial_blocked);
                auto trial_dist = bfs_dist(trial_adj, seeds, hops);
                double spread = simulate_consistent(trial_adj, seeds, trial_dist, use_hops, hops, sims_screen);
                double new_gain = celf_baseline - spread;
                heap.push({new_gain, top.cand_idx, iter});
            }
        }
        if (!found) break;
    }
    
    // Evaluate CELF solution
    double celf_spread = celf_baseline;
    if ((int)celf_list.size() == k) {
        celf_adj.build(max_node_id, edges, celf_blocked);
        celf_dist = bfs_dist(celf_adj, seeds, hops);
        celf_spread = simulate_consistent(celf_adj, seeds, celf_dist, use_hops, hops, sims_confirm);
    }
    
    cerr << "CELF result: spread=" << celf_spread << " (" << celf_list.size() << " edges)"
         << ", rho=" << (baseline_spread - celf_spread)/baseline_spread 
         << ", t=" << elapsed() << "s\n";
    
    // Pick the better solution between seed-edge and CELF
    if (celf_spread < best_spread && (int)celf_list.size() == k) {
        best_spread = celf_spread;
        blocked_list = celf_list;
        blocked_set = celf_blocked;
        write_output(output_path, blocked_list);
        cerr << "CELF solution is better, adopting it\n";
    } else {
        cerr << "Keeping seed-edge solution (spread=" << best_spread << ")\n";
    }
    
    // ====== PHASE 2: Local search refinement ======
    double local_deadline = time_limit - 30;
    cerr << "Phase 2: Local search, budget=" << local_deadline - elapsed() << "s\n";
    
    // Recalibrate sim counts for local search
    int sims_local = max(50, sims_screen);
    int sims_local_confirm = max(200, sims_confirm);
    
    // Build replacement candidate pool
    auto refresh_pool = [&](const unordered_set<EdgeKey>& bl) -> vector<CandEdge> {
        AdjList cur_adj;
        cur_adj.build(max_node_id, edges, bl);
        auto cands = rank_candidates(edges, seeds, hops, bl, cur_adj);
        int pool_size = min((int)cands.size(), max(500, k * 20));
        cands.resize(pool_size);
        return cands;
    };
    
    auto pool = refresh_pool(blocked_set);
    
    int total_swaps = 0;
    bool improved = true;
    
    while (improved && elapsed() < local_deadline) {
        improved = false;
        
        for (int bi = 0; bi < (int)blocked_list.size() && elapsed() < local_deadline; bi++) {
            auto [bu, bv] = blocked_list[bi];
            EdgeKey remove_key = make_key(bu, bv);
            
            // Try removing this edge — what's the spread?
            unordered_set<EdgeKey> trial_bl = blocked_set;
            trial_bl.erase(remove_key);
            
            AdjList trial_adj;
            trial_adj.build(max_node_id, edges, trial_bl);
            auto trial_dist = bfs_dist(trial_adj, seeds, hops);
            double spread_without = simulate_consistent(trial_adj, seeds, trial_dist, use_hops, hops, sims_local);
            
            // Try each candidate replacement
            int best_repl = -1;
            double best_repl_spread = spread_without;
            
            int max_try = min((int)pool.size(), 150);
            for (int ri = 0; ri < max_try && elapsed() < local_deadline; ri++) {
                auto& rc = pool[ri];
                EdgeKey add_key = make_key(rc.u, rc.v);
                if (trial_bl.find(add_key) != trial_bl.end()) continue;
                if (add_key == remove_key) continue;
                
                unordered_set<EdgeKey> swap_bl = trial_bl;
                swap_bl.insert(add_key);
                
                AdjList swap_adj;
                swap_adj.build(max_node_id, edges, swap_bl);
                auto swap_dist = bfs_dist(swap_adj, seeds, hops);
                double swap_spread = simulate_consistent(swap_adj, seeds, swap_dist, use_hops, hops, sims_local);
                
                if (swap_spread < best_repl_spread - 0.05) {
                    best_repl_spread = swap_spread;
                    best_repl = ri;
                }
            }
            
            // Confirm improvement with more sims
            if (best_repl >= 0 && best_repl_spread < best_spread - 0.1) {
                auto& rc = pool[best_repl];
                EdgeKey add_key = make_key(rc.u, rc.v);
                unordered_set<EdgeKey> swap_bl = trial_bl;
                swap_bl.insert(add_key);
                
                AdjList swap_adj;
                swap_adj.build(max_node_id, edges, swap_bl);
                auto swap_dist = bfs_dist(swap_adj, seeds, hops);
                double confirmed = simulate_consistent(swap_adj, seeds, swap_dist, use_hops, hops, sims_local_confirm);
                
                if (confirmed < best_spread - 0.05) {
                    // Accept swap
                    blocked_set.erase(remove_key);
                    blocked_set.insert(add_key);
                    blocked_list[bi] = {rc.u, rc.v};
                    best_spread = confirmed;
                    write_output(output_path, blocked_list);
                    improved = true;
                    total_swaps++;
                    
                    cerr << "  SWAP #" << total_swaps << ": (" << bu << "," << bv 
                         << ") -> (" << rc.u << "," << rc.v 
                         << "), spread=" << confirmed << ", t=" << elapsed() << "s\n";
                }
            }
        }
        
        // Refresh pool after each full pass
        if (improved && elapsed() < local_deadline) {
            pool = refresh_pool(blocked_set);
        }
    }
    
    cerr << "Local search done, " << total_swaps << " swaps, t=" << elapsed() << "s\n";
    
    // Final evaluation with high sim count
    {
        AdjList final_adj;
        final_adj.build(max_node_id, edges, blocked_set);
        auto final_dist = bfs_dist(final_adj, seeds, hops);
        double final_spread = simulate_consistent(final_adj, seeds, final_dist, use_hops, hops, 2000);
        cerr << "Final spread (2000 sims): " << final_spread 
             << ", rho=" << (baseline_spread - final_spread)/baseline_spread << "\n";
    }
    
    write_output(output_path, blocked_list);
    return blocked_list;
}

// ─────────────────────────────────────────────
// 8. MAIN
// ─────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "Usage: ./forest_fire <graph> <seeds> <output> <k> <n_sims> <hops>\n";
        return 1;
    }
    
    t0 = chrono::steady_clock::now();
    
    string graph_path = argv[1];
    string seed_path = argv[2];
    string output_path = argv[3];
    int k = stoi(argv[4]);
    int n_sims = stoi(argv[5]);
    int hops = stoi(argv[6]);
    
    auto [n, edges] = load_graph(graph_path);
    vector<int> seeds = load_seeds(seed_path);
    
    cerr << "Graph: " << n << " nodes, " << edges.size() << " edges\n";
    cerr << "Seeds: " << seeds.size() << " initial burning regions\n";
    cerr << "k=" << k << ", n_sims=" << n_sims << ", hops=" << hops << "\n";
    
    double time_limit = 3500.0; // ~58 min, leaving safety margin
    
    auto result = solve(edges, seeds, k, n_sims, hops, output_path, time_limit);
    write_output(output_path, result);
    
    cerr << "Done. Blocked " << result.size() << " routes. Total: " << elapsed() << "s\n";
    return 0;
}
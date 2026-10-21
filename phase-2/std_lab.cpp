#include <algorithm>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;
    std::vector<int> edges;
};

CSRGraph LoadGraph(const char *filename)
{
    std::ifstream input(filename);
    if (!input) {
        throw std::runtime_error("Failed to open graph file");
    }

    std::vector<std::pair<int, int>> edge_list;
    std::string line;
    int max_vertex = -1;

    while (std::getline(input, line)) {
        std::istringstream iss(line);
        std::string first;

        // Skip blank lines and comments.
        if (!(iss >> first) || first[0] == '#') {
            continue;
        }

        int src = std::stoi(first);
        int dst;

        if (!(iss >> dst)) {
            throw std::runtime_error("Invalid edge-list line: " + line);
        }

        if (src < 0 || dst < 0) {
            throw std::runtime_error("Negative vertex ID");
        }

        edge_list.emplace_back(src, dst);
        max_vertex = std::max(max_vertex, std::max(src, dst));
    }

    int num_vertices = max_vertex + 1;

    CSRGraph graph;
    graph.num_vertices = num_vertices;
    graph.offsets.assign(num_vertices + 1, 0);
    graph.edges.resize(edge_list.size());

    // Count outgoing edges.
    for (const auto &[src, dst] : edge_list) {
        ++graph.offsets[src + 1];
    }

    // Prefix sum.
    for (int v = 0; v < num_vertices; ++v) {
        graph.offsets[v + 1] += graph.offsets[v];
    }

    // Populate destinations.
    std::vector<int> cursor = graph.offsets;

    for (const auto &[src, dst] : edge_list) {
        graph.edges[cursor[src]++] = dst;
    }

    return graph;
}


class Bfs : public BspAlgorithm {
public:
    Bfs(uint32_t num_vertices, uint32_t source, int nthreads) {
        // TODO: initialize the distance arrays and update flags.
        dist_[source] = 0;
        dist_prev_[source] = 0;
    }

    bool HasWork() const override {
        // TODO: return whether another round should run.
        return any_updated_;
    }

    void Process(int tid, uint32_t v, const CsrGraph& g) override {
        // TODO: update vertex v using its incoming neighbors' snapshot values.
        if (dist_prev_ < 0) return;

        int next = dist_prev_ + 1;

        for (int i = g.offsets[v]; i < g.offsets[v + 1]; ++i) 
        {
            uint32_t neighbor = g.edges[i];

            if (dist_prev_[neighbor] == -1) {
                dist_[neighbor] = next_dist;
                tl_updated_[tid] = 1;
            }
        }
    }

    void PostRound() override {
        // TODO: combine and reset update flags, then update the snapshot.
                any_updated_ = false;

        for (uint8_t updated : tl_updated_) {
            if (updated) {
                any_updated_ = true;
                break;
            }
        }

        std::fill(tl_updated_.begin(), tl_updated_.end(), 0);

        dist_prev_ = dist_;
    }

    int distance(uint32_t v) const {
        // TODO: return the hop distance to v, or -1 if unreachable.
        return dist_[v];
    }

private:
    std::vector<int> dist_;             // current hop distances
    std::vector<int> dist_prev_;        // previous-round snapshot
    std::vector<uint8_t> tl_updated_;   // one update flag per thread; serial uses index 0
    bool any_updated_ = true;          // run the first round
};

int main()
{
    CSRGraph g = LoadGraph("soc-Slashdot0902.txt");
    Bfs bfs(g.num_vertices, /*source=*/0, /*nthreads=*/1);
    BspSerial(g, bfs);  
    for (uint32_t v = 0; v < static_cast<uint32_t>(g.num_vertices); ++v) 
    {
        std::cout << "Distance to " << v << ": " << bfs.distance(v) << '\n';
    }
    return 0;
}
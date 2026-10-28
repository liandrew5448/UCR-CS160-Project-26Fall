// g++ -std=c++20 -pthread std_thread.cpp -o std_thread && ./std_thread

#include <chrono>
#include <iostream>
#include <thread>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;
    std::vector<int> edges;
};

CSRGraph LoadGraph(const char *filename)
{
  std::ifstream input(filename);
  std::string line;
  int max_vertex = -1;
  std::vector<std::pair<int, int>> edges;

  while (std::getline(input, line)) 
  {
      // Skip comments, then parse src and dst from this line.
      std::istringstream iss(line);
      iss >> std::ws;

      if(line[0] == '#' || line.empty()) {

          continue;
      }

      int src, dst;

      if(!(iss >> src >> dst)) 
      {
          std::cerr << "Error parsing line: " << line << std::endl;
          continue;
      }

      edges.emplace_back(src, dst);
      max_vertex = std::max(max_vertex, std::max(src, dst));
  }
  
  CSRGraph graph;
  graph.num_verticies = max_vertex + 1;
  graph.offset.resize(graph.num_verticies + 1, 0);

  for(const auto& edge : edges) 
  {
      graph.offset[edge.first + 1]++;
  }

  for(int i = 1; i <= graph.num_verticies; ++i) 
  {
      graph.offset[i] += graph.offset[i - 1];
  }

  graph.edges.resize(edges.size());
  std::vector<int> current_offset = graph.offset;

  for(const auto& edge : edges)
  {
      int src = edge.first;
      int dst = edge.second;
      graph.edges[current_offset[src]++] = dst;
  }

  return graph;

}

void worker(int id) {
  for (int i = 0; i < 5; ++i) {
    std::cout << "std::thread worker " << id << ", iteration " << i
              << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}

int main() {
  std::thread t1(worker, 1);
  std::thread t2(worker, 2);

  t1.join();
  t2.join();

  std::cout << "All std::thread workers finished." << std::endl;
  return 0;
}
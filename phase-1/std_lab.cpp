#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <iostream>
#include <algorithm>
#include <stdexcept>

struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;
    std::vector<int> edges;
};

using QueryCallback = std::function<std::string(const CSRGraph&, int src, int K)>;

struct QueryTask {
    int src;
    int K;
    QueryCallback cb;
    std::string result;
};

void RunTasksParallel(const CSRGraph& g, std::vector<QueryTask>& tasks, int num_threads)
{
    if(tasks.empty() || num_threads <= 0) {
        return;
    }

    if(num_threads > tasks.size()) {
        num_threads = tasks.size();
    }

    //create workers
    std::vector<std::thread> worker;
    worker.reserve(num_threads);

    //create threads
    for(int i = 0; i < num_threads; i++)
    {
        int start = (tasks.size()/num_threads) * i;
        int end = (tasks.size()/num_threads) * (i + 1);
        worker.emplace_back([&, i]()
        {
            for(int i = start; i < end; i++)
            {
                tasks[i].result = tasks[i].cb(g, tasks[i].src, tasks[i].K);
            }
        });
    }


    //join all
    for(auto& t : worker)
    {
        t.join();
    }
    return;
};



int main()
{
                      
    CSRGraph g{4, {0, 2, 3, 4, 4}, {1, 2, 3, 3}};

    QueryCallback out_degree = [](const CSRGraph& graph, int src, int) {
        return std::to_string(graph.offsets[src + 1] - graph.offsets[src]);
    };

    std::vector<QueryTask> tasks;
    for (int src = 0; src < 4; ++src) {
        tasks.push_back({src, 0, out_degree, ""});
    }

    RunTasksParallel(g, tasks, 2);

    for(int i = 0; i <tasks.size(); i++)
    {
        std::cout << tasks[i].result << " ";
    }
}

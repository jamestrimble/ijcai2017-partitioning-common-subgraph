#include <limits.h>
#include <stdbool.h>

#include <vector>

struct Graph {
    int n;
    std::vector<std::vector<unsigned char>> adjmat;
    std::vector<std::vector<unsigned char>> antiadjmat;
    std::vector<unsigned int> label;
    Graph(unsigned int n);
};

Graph readGraph(char* filename, char format);


#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <sstream>

int read_int(std::string s) {
    std::istringstream ss(s);
    int retval;
    if (!(ss >> retval))
        return 0;
    return retval;
}

void write_word(int word, std::ofstream &fout) {
    unsigned char bytes[2];
    bytes[0] = word & 0xFF;
    bytes[1] = (word>>8) & 0xFF;
    fout.write((char *) bytes, 2);
}

void generate(int n,
              int p,
              int min_vtx_label,
              int max_vtx_label,
              int min_edge_label,
              int max_edge_label,
              int loops_policy,
              std::string filename)
{
    std::ofstream fout;
    fout.open(filename, std::ios::binary | std::ios::out);

    write_word(n, fout);

    std::random_device seeder;
    std::mt19937 rng(seeder());
    std::uniform_int_distribution<int> vtx_label_gen(min_vtx_label, max_vtx_label);
    std::uniform_int_distribution<int> edge_label_gen(min_edge_label, max_edge_label);
    std::uniform_int_distribution<int> edge_exists_dist(1, 100);

    for (int i=0; i<n; i++)
        write_word(vtx_label_gen(rng), fout);

    for (int i=0; i<n; i++) {
        std::vector<std::pair<int, int>> edges;    // (target, label) pairs
        for (int j=0; j<n; j++) {
            if (i != j) {
                if (edge_exists_dist(rng) <= p) {
                    edges.push_back({j, edge_label_gen(rng)});
                }
            }
        }
        if (loops_policy==2 || (loops_policy==1 && i%2)) {
            edges.push_back({i, 1});
        }
        write_word(edges.size(), fout);
        for (auto edge : edges) {
            write_word(edge.first, fout);   // target
            write_word(edge.second, fout);  // label
        }
    }

    fout.close();
}

int main(int argc, char **argv) {
    if (argc != 9) {
        std::cerr << "Usage: " << argv[0];
        std::cerr << " n p min_vtx_label max_vtx_label min_edge_label max_edge_label loops_policy filename" << std::endl;
        std::cerr << "Where p is an integer from 0 to 100" << std::endl;
        std::cerr << "and loops_policy is 0 for no loops, 1 for 50% loops, and 2 for all loops" << std::endl;
        return 1;
    }
    
    int i = 1;
    int n = read_int(argv[i++]);
    int p = read_int(argv[i++]);
    int min_vtx_label = read_int(argv[i++]);
    int max_vtx_label = read_int(argv[i++]);
    int min_edge_label = read_int(argv[i++]);
    int max_edge_label = read_int(argv[i++]);
    int loops_policy = read_int(argv[i++]);
    std::string filename = argv[i++];

    generate(n, p, min_vtx_label, max_vtx_label, min_edge_label, max_edge_label, loops_policy, filename);
}


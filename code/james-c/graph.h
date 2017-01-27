#define MAX_N 7001 

#include <limits.h>
#include <stdbool.h>

#define BITS_PER_UNSIGNED_INT (CHAR_BIT * sizeof(unsigned int))

struct Graph {
    int n;
    int degree[MAX_N];
    unsigned char adjmat[MAX_N][MAX_N];
    unsigned int label[MAX_N];
};

// Precondition: *g is already zeroed out
void readGraph(char* filename, struct Graph* g, bool directed, bool labelled);

// Precondition: *g is already zeroed out
void readBinaryGraph(char* filename, struct Graph* g, bool directed, bool labelled);

// Precondition: *g is already zeroed out
void readLadGraph(char* filename, struct Graph* g, bool directed);

void induced_subgraph(struct Graph *g, struct Graph *subg, int *vv, int vv_len);

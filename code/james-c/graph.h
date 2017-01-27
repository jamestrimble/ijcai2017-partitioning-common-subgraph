#define MAX_N 7001 

#include <limits.h>
#include <stdbool.h>

#define BITS_PER_UNSIGNED_INT (CHAR_BIT * sizeof(unsigned int))

#ifdef LABELLED
typedef unsigned int edge_label_t;
#else
typedef unsigned char edge_label_t;
#endif

struct Graph {
    int n;
    int degree[MAX_N];
    edge_label_t adjmat[MAX_N][MAX_N];
    unsigned int label[MAX_N];
};

// Precondition: *g is already zeroed out
void readGraph(char* filename, struct Graph* g, bool directed, bool labelled);

// Precondition: *g is already zeroed out
void readBinaryGraph(char* filename, struct Graph* g, bool directed, bool labelled);

// Precondition: *g is already zeroed out
void readLadGraph(char* filename, struct Graph* g, bool directed);

void induced_subgraph(struct Graph *g, struct Graph *subg, int *vv, int vv_len);

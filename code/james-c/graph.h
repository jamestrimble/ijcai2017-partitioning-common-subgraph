#define MAX_N 7001 

#include <limits.h>
#include <stdbool.h>

#define BITS_PER_UNSIGNED_INT (CHAR_BIT * sizeof(unsigned int))

#ifdef LABELLED
typedef unsigned int edge_label_t;
#else
typedef unsigned char edge_label_t;
#endif

enum graph_format {DIMACS_FORMAT, VF_FORMAT, LAD_FORMAT};

struct Graph {
    int n;
    int degree[MAX_N];
    edge_label_t adjmat[MAX_N][MAX_N];
    unsigned int label[MAX_N];
};

struct Graph *read_graph(char* filename, enum graph_format format, bool directed, bool labelled);

struct Graph *induced_subgraph(struct Graph *g, int *vv, int vv_len);

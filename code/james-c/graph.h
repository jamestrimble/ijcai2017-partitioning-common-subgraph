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
    int *degree;                    // the degree of each vertex
    edge_label_t **adjmat;          // each element points to a row of the adjacency matrix
    unsigned int *label;            // a label for each vertex
    edge_label_t *adjmat_elements;  // a flat array containing the n*n elements of the adj. matrix
};

struct Graph *new_graph(int n);

void free_graph(struct Graph *g);

struct Graph *read_graph(char* filename, enum graph_format format, bool directed, bool labelled);

struct Graph *induced_subgraph(struct Graph *g, int *vv, int vv_len);

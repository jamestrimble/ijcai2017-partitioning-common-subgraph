#define MAX_N 7001 

#include <limits.h>
#include <stdbool.h>

typedef unsigned long long ULL;

struct Graph {
    int n;
    unsigned char adjmat[MAX_N][MAX_N];
    unsigned int label[MAX_N];
};

// Precondition: *g is already zeroed out
void readGraph(char* filename, struct Graph* g, char format);

// Precondition: *g is already zeroed out
int readBinaryGraph(char* filename, struct Graph* g);

// Precondition: *g is already zeroed out
void readLadGraph(char* filename, struct Graph* g);


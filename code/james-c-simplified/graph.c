#define _GNU_SOURCE

#include "graph.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void fail(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

void add_edge(struct Graph *g, int v, int w) {
    if (v != w) {
        g->adjmat[v][w] = 1;
        g->adjmat[w][v] = 1;
    } else {
        // To indicate that a vertex has a loop, we set its label to 1
        g->label[v] = 1;
    }
}

// Precondition: *g is already zeroed out
void readDimacsGraph(char* filename, struct Graph* g) {
    FILE* f;
    
    if ((f=fopen(filename, "r"))==NULL)
        fail("Cannot open file");

    char* line = NULL;
    size_t nchar = 0;

    int nvertices = 0;
    int medges = 0;
    int v, w;
    int edges_read = 0;

    while (getline(&line, &nchar, f) != -1) {
        if (nchar > 0) {
            switch (line[0]) {
            case 'p':
                if (sscanf(line, "p edge %d %d", &nvertices, &medges)!=2)
                    fail("Error reading a line beginning with p.\n");
                printf("%d vertices\n", nvertices);
                if (nvertices >= MAX_N)
                    fail("Too many vertices. Please recompile with a larger MAX_N.\n");
                g->n = nvertices;
                break;
            case 'e':
                if (sscanf(line, "e %d %d", &v, &w)!=2)
                    fail("Error reading a line beginning with e.\n");
                add_edge(g, v-1, w-1);
                edges_read++;
                break;
            }
        }
    }

    if (medges>0 && edges_read != medges) fail("Unexpected number of edges.");

    fclose(f);
}

// Precondition: *g is already zeroed out
void readLadGraph(char* filename, struct Graph* g) {
    FILE* f;
    
    if ((f=fopen(filename, "r"))==NULL)
        fail("Cannot open file");

    int nvertices = 0;
    int w;

    if (fscanf(f, "%d", &nvertices) != 1)
        fail("Number of vertices not read correctly.\n");
    if (nvertices >= MAX_N)
        fail("Too many vertices. Please recompile with a larger MAX_N.\n");
    g->n = nvertices;

    for (int i=0; i<nvertices; i++) {
        int edge_count;
        if (fscanf(f, "%d", &edge_count) != 1)
            fail("Number of edges not read correctly.\n");
        for (int j=0; j<edge_count; j++) {
            if (fscanf(f, "%d", &w) != 1)
                fail("An edge was not read correctly.\n");
            add_edge(g, i, w);
        }
    }

    fclose(f);
}

int read_word(FILE *fp) {
    unsigned char a[2];
    if (fread(a, 1, 2, fp) != 2)
        fail("Error reading file.\n");
    return (int)a[0] | (((int)a[1]) << 8);
}

// Precondition: *g is already zeroed out
// returns max edge label
int readBinaryGraph(char* filename, struct Graph* g) {
    FILE* f;
    
    if ((f=fopen(filename, "rb"))==NULL)
        fail("Cannot open file");

    int nvertices = read_word(f);
    g->n = nvertices;
    printf("%d vertices\n", nvertices);

    // Labelling scheme: see
    // https://github.com/ciaranm/cp2016-max-common-connected-subgraph-paper/blob/master/code/solve_max_common_subgraph.cc
    int m = g->n * 33 / 100;
    int p = 1;
    int k1 = 0;
    int k2 = 0;
    while (p < m && k1 < 16) {
        p *= 2;
        k1 = k2;
        k2++;
    }

    for (int i=0; i<nvertices; i++) {
        read_word(f);   // ignore label
    }

    int max_label = 0;
    for (int i=0; i<nvertices; i++) {
        int len = read_word(f);
        for (int j=0; j<len; j++) {
            int target = read_word(f);
            read_word(f); // ignore label
            add_edge(g, i, target);
        }
    }
    fclose(f);
    return max_label;
}

void readGraph(char* filename, struct Graph* g, char format) {
    if (format=='D') readDimacsGraph(filename, g);
    else if (format=='L') readLadGraph(filename, g);
    else if (format=='B') readBinaryGraph(filename, g);
    else fail("Unknown graph format\n");
}

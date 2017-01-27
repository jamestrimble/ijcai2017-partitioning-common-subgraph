#define _GNU_SOURCE

#include "graph.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void fail(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

// Unlabelled, undirected: if there is an edge v-w, then adjmat[v][w]==adjmat[w][v]==1
// Labelled, undirected:   if there is an edge v-w, then adjmat[v][w]==adjmat[w][v]==label
// Unlabelled, directed:   if there is an edge v->w, then adjmat[v][w]&1 == (adjmat[w][v]>>1)&1 == 1
// Labelled, directed:     if there is an edge v->w, then the least significant two bytes of adjmat[v][w]
//                          and the most significant two bytes of adjmat[w][v] equal the label
void add_edge(struct Graph *g, int v, int w, edge_label_t edge_val, bool directed) {
    if (v != w) {
#ifdef LABELLED
        if (edge_val > 0xFFFFu)
            fail("An edge label is too large.");
        if (directed) {
            if (g->adjmat[v][w] & 0xFFFFu)
                fail("Duplicate edge.");
            g->adjmat[v][w] |= edge_val;
            g->adjmat[w][v] |= (edge_val << 16);
        } else {
            g->adjmat[v][w] = edge_val;
            g->adjmat[w][v] = edge_val;
        }
#else
        if (directed) {
            g->adjmat[v][w] |= 1;
            g->adjmat[w][v] |= 2;
        } else {
            g->adjmat[v][w] = 1;
            g->adjmat[w][v] = 1;
        }
#endif
    } else if (edge_val != 1) {
        fail("Unexpected edge val != 1 on a loop\n");
    } else {
        // To indicate that a vertex has a loop, we set the most significant bit
        // of its label to 1
        g->label[v] |= (1u << (BITS_PER_UNSIGNED_INT-1));
    }
}

void induced_subgraph(struct Graph *g, struct Graph *subg, int *vv, int vv_len) {
    subg->n = vv_len;
    for (int i=0; i<subg->n; i++)
        for (int j=0; j<subg->n; j++)
            subg->adjmat[i][j] = g->adjmat[vv[i]][vv[j]];

    for (int i=0; i<subg->n; i++)
        subg->label[i] = g->label[vv[i]];
}

// Precondition: *g is already zeroed out
void readGraph(char* filename, struct Graph* g, bool directed, bool labelled) {
    FILE* f;
    
    if ((f=fopen(filename, "r"))==NULL)
        fail("Cannot open file");

    char* line = NULL;
    size_t nchar = 0;

    int nvertices = 0;
    int medges = 0;
    int v, w;
    int edges_read = 0;
    int label;

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
                add_edge(g, v-1, w-1, 1, directed);
                edges_read++;
                break;
            case 'n':
                if (sscanf(line, "n %d %d", &v, &label)!=2)
                    fail("Error reading a line beginning with n.\n");
                if (labelled)
                    g->label[v-1] |= label;
                break;
            }
        }
    }

    if (medges>0 && edges_read != medges) fail("Unexpected number of edges.");

    fclose(f);
}

// Precondition: *g is already zeroed out
void readLadGraph(char* filename, struct Graph* g, bool directed) {
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
            add_edge(g, i, w, 1, directed);
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
void readBinaryGraph(char* filename, struct Graph* g, bool directed, bool labelled) {
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
        int label = (read_word(f) >> (16-k1));
        if (labelled)
            g->label[i] |= label;
    }

    for (int i=0; i<nvertices; i++) {
        int len = read_word(f);
        for (int j=0; j<len; j++) {
            int target = read_word(f);
            int label = (read_word(f) >> (16-k1)) + 1;
            add_edge(g, i, target, labelled ? label : 1, directed);
        }
    }
    fclose(f);
}

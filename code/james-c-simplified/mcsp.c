#define _GNU_SOURCE
#define _POSIX_SOURCE

#include "graph.h"

#include <argp.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef unsigned long long ULL;

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}
static void fail(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

/*******************************************************************************
                             Command-line arguments
*******************************************************************************/

static char doc[] = "Find a maximum clique in a graph in DIMACS format";
static char args_doc[] = "FILENAME1 FILENAME2";
static struct argp_option options[] = {
    {"quiet", 'q', 0, 0, "Quiet output"},
    {"verbose", 'v', 0, 0, "Verbose output"},
    {"dimacs", 'd', 0, 0, "Read DIMACS format"},
    {"lad", 'l', 0, 0, "Read LAD format"},
    {"connected", 'c', 0, 0, "Solve max common CONNECTED subgraph problem"},
    { 0 }
};

static struct {
    bool quiet;
    bool verbose;
    bool connected;
    bool dimacs;
    bool lad;
    char *filename1;
    char *filename2;
    int arg_num;
} arguments;

void set_default_arguments() {
    arguments.quiet = false;
    arguments.verbose = false;
    arguments.connected = false;
    arguments.dimacs = false;
    arguments.lad = false;
    arguments.filename1 = NULL;
    arguments.filename2 = NULL;
    arguments.arg_num = 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'd':
            if (arguments.lad)
                fail("The -d and -l options cannot be used together.\n");
            arguments.dimacs = true;
            break;
        case 'l':
            if (arguments.dimacs)
                fail("The -d and -l options cannot be used together.\n");
            arguments.lad = true;
            break;
        case 'q':
            arguments.quiet = true;
            break;
        case 'v':
            arguments.verbose = true;
            break;
        case 'c':
            arguments.connected = true;
            break;
        case ARGP_KEY_ARG:
            if (arguments.arg_num == 0) {
                arguments.filename1 = arg;
            } else if (arguments.arg_num == 1) {
                arguments.filename2 = arg;
            } else {
                argp_usage(state);
            }
            arguments.arg_num++;
            break;
        case ARGP_KEY_END:
            if (arguments.arg_num == 0)
                argp_usage(state);
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

/*******************************************************************************
                                     Stats
*******************************************************************************/

static long nodes = 0;

/*******************************************************************************
                                 MCS functions
*******************************************************************************/

struct VtxPair {
    int v;
    int w;
};

struct VtxPairList {
    struct VtxPair vals[MAX_N];
    int len;
};

struct Bidomain {
    int *left_vv,     *right_vv;
    int  left_len,     right_len;
    bool is_adjacent;
};

struct BidomainList {
    struct Bidomain vals[MAX_N*2];
    int len;
};

struct D {
    struct Graph *g0;
    struct Graph *g1;
    struct VtxPairList *incumbent;
    struct VtxPairList *current;
    struct BidomainList *domains;
};

struct BidomainList preallocated_lists[MAX_N];

int calc_bound(struct BidomainList *domains) {
    int bound = 0;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        bound += MIN(bd->left_len, bd->right_len);
    }
    return bound;
}

struct Bidomain *select_bidomain(struct BidomainList *domains, int current_matching_size) {
    // Select the bidomain with the smallest max(leftsize, rightsize), breaking
    // ties on the smallest vertex index in the left set
    int min_size = INT_MAX;
    struct Bidomain *best = NULL;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        if (arguments.connected && current_matching_size>0 && !bd->is_adjacent) continue;
        int len = MAX(bd->left_len, bd->right_len);
        if (len < min_size) {
            min_size = len;
            best = bd;
        }
    }
    return best;
}

// Returns length of left half of array
int partition(int *vv, int vv_len, unsigned char *adjrow) {
    int i=0;
    for (int j=0; j<vv_len; j++) {
        int adj = adjrow[vv[j]];
        if (adj) {
            swap(&vv[i], &vv[j]);
            i++;
        }
    }
    return i;
}

void add_bidomain(struct BidomainList *bd_list, int *left_vv, int *right_vv,
        int left_len, int right_len, bool is_adjacent)
{
    bd_list->vals[bd_list->len++] = (struct Bidomain) {
        .left_vv=left_vv,
        .right_vv=right_vv,
        .left_len=left_len,
        .right_len=right_len,
        .is_adjacent=is_adjacent
    };
}

void filter_domains(struct BidomainList *d, struct BidomainList *new_d,
        struct Graph *g0, struct Graph *g1, int v, int w)
{
    new_d->len=0;
    for (int j=0; j<d->len; j++) {
        struct Bidomain *old_bd = &d->vals[j];
        int left_len_edge = partition(old_bd->left_vv, old_bd->left_len, g0->adjmat[v]);
        int right_len_edge = partition(old_bd->right_vv, old_bd->right_len, g1->adjmat[w]);
        int left_len_noedge = old_bd->left_len - left_len_edge;
        int right_len_noedge = old_bd->right_len - right_len_edge;
        if (left_len_noedge && right_len_noedge) {
            int *left_vv = old_bd->left_vv + left_len_edge;
            int *right_vv = old_bd->right_vv + right_len_edge;
            add_bidomain(new_d, left_vv, right_vv, left_len_noedge, right_len_noedge,
                    old_bd->is_adjacent);
        }
        if (left_len_edge && right_len_edge) {
            add_bidomain(new_d, old_bd->left_vv, old_bd->right_vv,
                    left_len_edge, right_len_edge, true);
        }
    }
}

void remove_bidomain(struct BidomainList *list, struct Bidomain *b) {
    int i = b - &list->vals[0];
    list->vals[i] = list->vals[list->len-1];
    list->len--;
}

void show(struct D d) {
    printf("Nodes: %ld\n", nodes);
    printf("Length of current assignment: %d\n", d.current->len);
    printf("Current assignment:");
    for (int i=0; i<d.current->len; i++) {
        printf("  %d->%d", d.current->vals[i].v, d.current->vals[i].w);
    }
    printf("\n");
    for (int i=0; i<d.domains->len; i++) {
        struct Bidomain bd = d.domains->vals[i];
        printf("Left  ");
        for (int j=0; j<bd.left_len; j++)
            printf("%d ", bd.left_vv[j]);
        printf("\n");
        printf("Right  ");
        for (int j=0; j<bd.right_len; j++)
            printf("%d ", bd.right_vv[j]);
        printf("\n");
    }
    printf("\n\n");
}

void set_incumbent(struct VtxPairList *current, struct VtxPairList *incumbent) {
    incumbent->len = current->len;
    for (int i=0; i<current->len; i++)
        incumbent->vals[i] = current->vals[i];
}

int find_and_remove_min_value(int *arr, int *len) {
    int min_v = INT_MAX;
    int idx = -1;
    for (int i=0; i<*len; i++) {
        if (arr[i] < min_v) {
            min_v = arr[i];
            idx = i;
        }
    }
    swap(&arr[idx], &arr[*len-1]);
    (*len)--;
    return min_v;
}

// returns the index of the smallest value in arr that is >w.
// Assumption: such a value exists
// Assumption: arr contains no duplicates
// Assumption: arr has no values==INT_MAX
int index_of_next_smallest(int *arr, int len, int w) {
    int idx = -1;
    int smallest = INT_MAX;
    for (int i=0; i<len; i++) {
        if (arr[i]>w && arr[i]<smallest) {
            smallest = arr[i];
            idx = i;
        }
    }
    return idx;
}

void solve(struct D d, int level) {
    if (arguments.verbose) show(d);

    nodes++;

    if (d.current->len > d.incumbent->len) {
        set_incumbent(d.current, d.incumbent);
        if (!arguments.quiet) printf("Incumbent size: %d\n", d.incumbent->len);
    }

    if (d.current->len + calc_bound(d.domains) <= d.incumbent->len)
        return;

    struct Bidomain *bd = select_bidomain(d.domains, d.current->len);
    if (bd == NULL)   // In the MCCS case, there may be nothing we can branch on
        return;
    struct Bidomain bd_copy;

    int v = find_and_remove_min_value(bd->left_vv, &bd->left_len);
    if (bd->left_len == 0) {
        bd_copy = *bd;
        remove_bidomain(d.domains, bd);
        bd = &bd_copy;
    }

    struct D new_d = d;
    new_d.domains = &preallocated_lists[level];

    // Try assigning v to each vertex w in bd->right_vv, in turn
    bd->right_len--;
    int w = -1;
    for (int i=0; i<=bd->right_len; i++) {
        int idx = index_of_next_smallest(bd->right_vv, bd->right_len+1, w);
        w = bd->right_vv[idx];
        
        // swap w with the value just past the end of the right_vv array
        bd->right_vv[idx] = bd->right_vv[bd->right_len];
        bd->right_vv[bd->right_len] = w;

        filter_domains(d.domains, new_d.domains, d.g0, d.g1, v, w);
        d.current->vals[d.current->len++] = (struct VtxPair) {.v=v, .w=w};
        solve(new_d, level+1);
        d.current->len--;
    }
    bd->right_len++;
    solve(d, level+1);
}

struct VtxPairList mcs(struct Graph *g0, struct Graph *g1) {
    int incumbent_size = 0;
    struct VtxPairList incumbent = {.len=incumbent_size};
    struct BidomainList *domains = &preallocated_lists[0];

    int left[MAX_N];  // the buffer of vertex indices for the left partitions
    int right[MAX_N];  // the buffer of vertex indices for the right partitions
    int l = 0;  // next free index in left
    int r = 0;  // next free index in right

    // Create a bidomain for vertices without loops (label 0),
    // and another for vertices with loops (label 1)
    for (int label=0; label<=1; label++) {
        int start_l = l;
        int start_r = r;

        for (int i=0; i<g0->n; i++)
            if (g0->label[i]==label)
                left[l++] = i;
        for (int i=0; i<g1->n; i++)
            if (g1->label[i]==label)
                right[r++] = i;

        int left_len = l - start_l;
        int right_len = r - start_r;
        if (left_len && right_len)
            add_bidomain(domains, &left[start_l], &right[start_r], left_len, right_len, false);
    }

    struct D d = {.g0=g0, .g1=g1, .incumbent=&incumbent,
           .current=&(struct VtxPairList){.len=0}, .domains=domains};

    solve(d, 1);

    return incumbent;
}

bool check_sol(struct Graph *g0, struct Graph *g1, struct VtxPairList *solution) {
    bool used_left[MAX_N] = {false};
    bool used_right[MAX_N] = {false};
    for (int i=0; i<solution->len; i++) {
        struct VtxPair p0 = solution->vals[i];
        if (used_left[p0.v] || used_right[p0.w])
            return false;
        used_left[p0.v] = true;
        used_right[p0.w] = true;
        if (g0->label[p0.v] != g1->label[p0.w])
            return false;
        for (int j=i+1; j<solution->len; j++) {
            struct VtxPair p1 = solution->vals[j];
            if (g0->adjmat[p0.v][p1.v] != g1->adjmat[p0.w][p1.w])
                return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);

    struct Graph* g0 = calloc(1, sizeof(*g0));
    struct Graph* g1 = calloc(1, sizeof(*g1));

    char format = arguments.dimacs ? 'D' : arguments.lad ? 'L' : 'B';
    readGraph(arguments.filename1, g0, format);
    readGraph(arguments.filename2, g1, format);

    clock_t start = clock();

    struct VtxPairList solution = mcs(g0, g1);

    clock_t time_elapsed = clock() - start;

    if (!check_sol(g0, g1, &solution))
        fail("*** Error: Invalid solution\n");

    printf("Solution size %d\n", solution.len);
    for (int i=0; i<g0->n; i++)
        for (int j=0; j<solution.len; j++)
            if (solution.vals[j].v == i)
                printf("(%d -> %d) ", solution.vals[j].v, solution.vals[j].w);
    printf("\n");

    setlocale(LC_NUMERIC, "");
    printf("Nodes:                      %'15ld\n", nodes);
    printf("CPU time (ms):              %15ld\n", time_elapsed * 1000 / CLOCKS_PER_SEC);

    free(g0);
    free(g1);
}

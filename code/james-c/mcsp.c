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

#define DEG_SEQ_CHECK_SIZE_LIMIT 15

#define INSERTION_SORT(type, arr, arr_len, swap_condition) do { \
    for (int i=1; i<arr_len; i++) {                             \
        for (int j=i; j>=1; j--) {                              \
            if (swap_condition) {                               \
                type tmp = arr[j-1];                            \
                arr[j-1] = arr[j];                              \
                arr[j] = tmp;                                   \
            } else {                                            \
                break;                                          \
            }                                                   \
        }                                                       \
    }                                                           \
} while(0);

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}
static void fail(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

clock_t start;

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
    {"timeout", 't', "TIMEOUT", 0, "Set timeout of TIMEOUT seconds"},
    {"connected", 'c', 0, 0, "Solve max common CONNECTED subgraph problem"},
    {"directed", 'r', 0, 0, "Solve max common CONNECTED subgraph problem"},
    {"big-first", 'b', 0, 0, "First try to find an induced subgraph isomorphism, then decrement the target size"},
    { 0 }
};

static struct {
    bool quiet;
    bool verbose;
    bool connected;
    bool directed;
    bool big_first;
    bool dimacs;
    bool lad;
    int timeout;
    char *filename1;
    char *filename2;
    int arg_num;
} arguments;

void set_default_arguments() {
    arguments.quiet = false;
    arguments.verbose = false;
    arguments.connected = false;
    arguments.directed = false;
    arguments.big_first = false;
    arguments.dimacs = false;
    arguments.lad = false;
    arguments.timeout = 0;
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
        case 'b':
            arguments.big_first = true;
            break;
        case 'v':
            arguments.verbose = true;
            break;
        case 'c':
            if (arguments.directed)
                fail("The connected and directed options cannot be used together.\n");
            arguments.connected = true;
            break;
        case 'r':
            if (arguments.connected)
                fail("The connected and directed options cannot be used together.\n");
            arguments.directed = true;
            break;
        case 't':
            arguments.timeout = atoi(arg);
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

static struct {
    long nodes;
} stats;

void initialise_stats() {
    stats.nodes = 0;
}

/*******************************************************************************
                                Graph functions
*******************************************************************************/

void calculate_all_degrees(struct Graph *g) {
    for (int v=0; v<g->n; v++) {
        g->degree[v] = 0;
        for (int w=0; w<g->n; w++) {
            g->degree[v] += (g->adjmat[v][w] != 0);
            if (arguments.directed)
                g->degree[v] += (g->adjmat[w][v] != 0);
        }
    }
}

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
    int matching_size_goal;
};

struct BidomainList *preallocated_lists;

int calc_bound(struct BidomainList *domains) {
    int bound = 0;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        bound += MIN(bd->left_len, bd->right_len);
    }
    return bound;
}

int find_min(int *vv, int len) {
    int min = 999999999;
    for (int i=0; i<len; i++)
        if (vv[i] < min)
            min = vv[i];
    return min;
}

struct Bidomain *select_bidomain(struct BidomainList *domains, int current_matching_size) {
    // Select the bidomain with the smallest max(leftsize, rightsize), breaking
    // ties on the smallest vertex index in the left set
    int min_size = 999999999;
    int min_idx = 999999999;
    struct Bidomain *best = NULL;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        if (arguments.connected && current_matching_size>0 && !bd->is_adjacent) continue;
        int len = MAX(bd->left_len, bd->right_len);
        int idx;
        if (len < min_size) {
            min_size = len;
            min_idx = find_min(bd->left_vv, bd->left_len);
            best = &domains->vals[i];
        } else if (len==min_size && (idx=find_min(bd->left_vv, bd->left_len))<min_idx) {
            min_idx = idx;
            best = &domains->vals[i];
        }
    }
    return best;
}

// Returns length of left half of array
int partition(int *vv, int vv_len, unsigned char *adjrow) {
    int i=0;
    for (int j=0; j<vv_len; j++) {
#ifdef LABELLED
        int adj = adjrow[vv[j]] != 0;
#else
        int adj = adjrow[vv[j]];
#endif
        if (adj) {
            swap(&vv[i], &vv[j]);
            i++;
        }
    }
    return i;
}

// Swaps everything with an edge of the correct label to the end of vv
// Returns length of left half of array
int labelled_partition(int *vv, int vv_len, unsigned char *adjrow, unsigned char label) {
    int i=0;
    for (int j=0; j<vv_len; j++) {
        int nonadj = adjrow[vv[j]] != label;
        if (nonadj) {
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

void filter_domains_undirected(struct BidomainList *d, struct BidomainList *new_d,
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
#ifdef LABELLED
        int left_len = left_len_edge;
        int right_len = right_len_edge;
        while (left_len && right_len) {
            unsigned char label = g0->adjmat[v][old_bd->left_vv[0]];
            int left_len_no = labelled_partition(old_bd->left_vv, left_len, g0->adjmat[v], label);
            int right_len_no = labelled_partition(old_bd->right_vv, right_len, g1->adjmat[w], label);
            int left_len_yes = left_len - left_len_no;
            int right_len_yes = right_len - right_len_no;
            if (left_len_yes && right_len_yes) {
                int *left_vv = old_bd->left_vv + left_len_no;
                int *right_vv = old_bd->right_vv + right_len_no;
                add_bidomain(new_d, left_vv, right_vv, left_len_yes, right_len_yes, true);
            }
            //printf("%d %d %d\n", left_len_no, left_len_yes, left_len);
            left_len = left_len_no;
            right_len = right_len_no;
        }
#else
        if (left_len_edge && right_len_edge) {
            add_bidomain(new_d, old_bd->left_vv, old_bd->right_vv,
                    left_len_edge, right_len_edge, true);
        }
#endif
    }
}

int directed_labelled_partition(int *vv, int vv_len, int v, unsigned char (*adjmat)[MAX_N], unsigned int label) {
    int i=0;
    for (int j=0; j<vv_len; j++) {
        int nonadj = (adjmat[v][vv[j]]<<CHAR_BIT | adjmat[vv[j]][v]) != label;
        if (nonadj) {
            swap(&vv[i], &vv[j]);
            i++;
        }
    }
    return i;
}

void filter_domains_directed(struct BidomainList *d, struct BidomainList *new_d,
        struct Graph *g0, struct Graph *g1, int v, int w)
{
    new_d->len=0;
    for (int j=0; j<d->len; j++) {
        struct Bidomain *old_bd = &d->vals[j];
        int left_len = old_bd->left_len;
        int right_len = old_bd->right_len;
        while (left_len && right_len) {
            unsigned int label = (g0->adjmat[v][old_bd->left_vv[0]]<<CHAR_BIT) |
                                  g0->adjmat[old_bd->left_vv[0]][v];
            int left_len_no = directed_labelled_partition(old_bd->left_vv, left_len, v, g0->adjmat, label);
            int right_len_no = directed_labelled_partition(old_bd->right_vv, right_len, w, g1->adjmat, label);
            int left_len_yes = left_len - left_len_no;
            int right_len_yes = right_len - right_len_no;
            if (left_len_yes && right_len_yes) {
                int *left_vv = old_bd->left_vv + left_len_no;
                int *right_vv = old_bd->right_vv + right_len_no;
                add_bidomain(new_d, left_vv, right_vv, left_len_yes, right_len_yes, true);
            }
            //printf("%d %d %d\n", left_len_no, left_len_yes, left_len);
            left_len = left_len_no;
            right_len = right_len_no;
        }
    }
}

void filter_domains(struct BidomainList *d, struct BidomainList *new_d,
        struct Graph *g0, struct Graph *g1, int v, int w)
{
    if (arguments.directed)
        filter_domains_directed(d, new_d, g0, g1, v, w);
    else
        filter_domains_undirected(d, new_d, g0, g1, v, w);
}

void remove_bidomain(struct BidomainList *list, struct Bidomain *b) {
    int i = b - &list->vals[0];
    list->vals[i] = list->vals[list->len-1];
    list->len--;
}

void show(struct D d) {
    printf("Nodes: %ld\n", stats.nodes);
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
    int min_v = 999999999;
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
// Assumption: arr has no values>=999999999
int index_of_next_smallest(int *arr, int len, int w) {
    int idx = -1;
    int smallest = 999999999;
    for (int i=0; i<len; i++) {
        if (arr[i]>w && arr[i]<smallest) {
            smallest = arr[i];
            idx = i;
        }
    }
    return idx;
}

void solve(struct D d, int level) {
//    printf(" mcsp --- Nodes: %ld\n", stats.nodes);
    if (arguments.verbose) show(d);

    stats.nodes++;
    if (arguments.timeout && stats.nodes%100000==0 && 
            (clock()-start)*1000/CLOCKS_PER_SEC > arguments.timeout*1000) {
        arguments.timeout = -1;
    }
    if (arguments.timeout == -1) return;

    if (d.current->len > d.incumbent->len) {
        set_incumbent(d.current, d.incumbent);
        if (!arguments.quiet) printf("Incumbent size: %d\n", d.incumbent->len);
    }

    int bound = d.current->len + calc_bound(d.domains);
    if (bound <= d.incumbent->len || bound < d.matching_size_goal)
        return;

    if (arguments.big_first && d.incumbent->len==d.matching_size_goal)
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
    new_d.domains->len = 0;

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

void sort_and_remove_dups(unsigned int *arr, int *len) {
    INSERTION_SORT(unsigned int, arr, *len, (arr[j-1] < arr[j]))
    int j=1;
    for (int i=1; i<*len; i++)
        if (arr[i] != arr[j-1])
            arr[j++] = arr[i];
    *len = j;
}

void build_domains_and_solve(struct Graph *g0, struct Graph *g1, struct VtxPairList *incumbent,
        int matching_size_goal, unsigned int *all_labels, int all_labels_len)
{
    struct BidomainList *domains = &preallocated_lists[0];
    domains->len = 0;

    int left[MAX_N];  // the buffer of vertex indices for the left partitions
    int right[MAX_N];  // the buffer of vertex indices for the right partitions
    int l = 0;  // next free index in left
    int r = 0;  // next free index in right
    for (int i=0; i<all_labels_len; i++) {
        int start_l = l;
        int start_r = r;

        unsigned int label = all_labels[i];

        for (int i=0; i<g0->n; i++)
            if (g0->label[i]==label)
                left[l++] = i;
        for (int i=0; i<g1->n; i++)
            if (g1->label[i]==label)
                right[r++] = i;

        int left_len = l - start_l;
        int right_len = r - start_r;
        if (left_len && right_len) {
            domains->vals[domains->len] = (struct Bidomain) {
                .left_vv = &left[start_l],
                .right_vv = &right[start_r],
                .left_len = left_len,
                .right_len = right_len
            };
            domains->len++;
        }
    }

    struct D d = {.g0=g0, .g1=g1, .incumbent=incumbent,
            .current=&(struct VtxPairList){.len=0}, .domains=domains,
            .matching_size_goal=matching_size_goal};

    solve(d, 1);
}

struct VtxPairList mcs(struct Graph *g0, struct Graph *g1) {
    unsigned int all_labels[MAX_N*2];
    int all_labels_len = 0;
    for (int i=0; i<g0->n; i++) all_labels[all_labels_len++] = g0->label[i];
    for (int i=0; i<g1->n; i++) all_labels[all_labels_len++] = g1->label[i];
    sort_and_remove_dups(all_labels, &all_labels_len);

    struct VtxPairList incumbent = {.len=0};

    if (arguments.big_first) {
        for (int k=0; k<g0->n; k++) {
            int goal = g0->n - k;
            build_domains_and_solve(g0, g1, &incumbent, goal, all_labels, all_labels_len);
            if (incumbent.len == goal) break;
            printf("Upper bound: %d\n", goal - 1);
        }
    } else {
        build_domains_and_solve(g0, g1, &incumbent, 1, all_labels, all_labels_len);
    }

    return incumbent;
}

// Count edges in the subgraph
int count_edges(struct Graph *g0, struct Graph *g1, struct VtxPairList *solution) {
    int count = 0;
    for (int i=0; i<solution->len; i++) {
        struct VtxPair p0 = solution->vals[i];
        for (int j=i+1; j<solution->len; j++) {
            struct VtxPair p1 = solution->vals[j];
            if (g0->adjmat[p0.v][p1.v]) count++;
            if (arguments.directed && g0->adjmat[p1.v][p0.v]) count++;
        }
    }
//    for (int i=0; i<solution->len; i++) {
//        struct VtxPair p0 = solution->vals[i];
//        for (int j=0; j<solution->len; j++) {
//            struct VtxPair p1 = solution->vals[j];
//            printf("%s ", g0->adjmat[p0.v][p1.v]!=0 ? "1" : ".");
//        }
//        printf("\n");
//    }
    return count;
}

bool check_sol(struct Graph *g0, struct Graph *g1, struct VtxPairList *solution) {
    bool used_left[MAX_N];
    bool used_right[MAX_N];
    for (int i=0; i<g0->n; i++) used_left[i] = false;
    for (int i=0; i<g1->n; i++) used_right[i] = false;
    for (int i=0; i<solution->len; i++) {
        struct VtxPair p0 = solution->vals[i];
        if (used_left[p0.v] || used_right[p0.w])
            return false;
        used_left[p0.v] = true;
        used_right[p0.w] = true;
        for (int j=i+1; j<solution->len; j++) {
            struct VtxPair p1 = solution->vals[j];
            if (g0->adjmat[p0.v][p1.v] != g1->adjmat[p0.w][p1.w]) return false;
            if (g0->adjmat[p1.v][p0.v] != g1->adjmat[p1.w][p0.w]) return false;
        }
    }
    return true;
}

int graph_edge_count(struct Graph *g) {
    int count = 0;
    for (int i=0; i<g->n; i++)
        count += g->degree[i];
    return count;
}

int main(int argc, char** argv) {
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);

    initialise_stats();

    struct Graph* g0 = calloc(1, sizeof(*g0));
    struct Graph* g1 = calloc(1, sizeof(*g1));

#ifdef LABELLED
    bool labelled = true;
#else
    bool labelled = false;
#endif

    if (arguments.dimacs) {
        readGraph(arguments.filename1, g0, arguments.directed, labelled);
        readGraph(arguments.filename2, g1, arguments.directed, labelled);
    } else if (arguments.lad) {
        if (labelled)
            fail("This program can't read LAD format with labels\n");
        readLadGraph(arguments.filename1, g0, arguments.directed);
        readLadGraph(arguments.filename2, g1, arguments.directed);
    } else {
        readBinaryGraph(arguments.filename1, g0, arguments.directed, labelled);
        readBinaryGraph(arguments.filename2, g1, arguments.directed, labelled);
    }

    start = clock();
    preallocated_lists = malloc((g0->n+1) * sizeof(struct BidomainList));
    calculate_all_degrees(g0);
    calculate_all_degrees(g1);

    int vv0[MAX_N];
    for (int i=0; i<g0->n; i++) vv0[i] = i;
    if (graph_edge_count(g1) > g1->n*(g1->n-1)/2) {
        INSERTION_SORT(int, vv0, g0->n, (g0->degree[vv0[j-1]] > g0->degree[vv0[j]]))
    } else {
        INSERTION_SORT(int, vv0, g0->n, (g0->degree[vv0[j-1]] < g0->degree[vv0[j]]))
    }

    int vv1[MAX_N];
    for (int i=0; i<g1->n; i++) vv1[i] = i;
    if (graph_edge_count(g0) > g0->n*(g0->n-1)/2) {
        INSERTION_SORT(int, vv1, g1->n, (g1->degree[vv1[j-1]] > g1->degree[vv1[j]]))
    } else {
        INSERTION_SORT(int, vv1, g1->n, (g1->degree[vv1[j-1]] < g1->degree[vv1[j]]))
    }

    struct Graph *g0_sorted = calloc(1, sizeof(struct Graph));
    struct Graph *g1_sorted = calloc(1, sizeof(struct Graph));
    induced_subgraph(g0, g0_sorted, vv0, g0->n);
    induced_subgraph(g1, g1_sorted, vv1, g1->n);
    calculate_all_degrees(g0_sorted);
    calculate_all_degrees(g1_sorted);

    struct VtxPairList solution = mcs(g0_sorted, g1_sorted);

    // Convert to indices from original, unsorted graphs
    for (int i=0; i<solution.len; i++) {
        solution.vals[i].v = vv0[solution.vals[i].v];
        solution.vals[i].w = vv1[solution.vals[i].w];
    }

    clock_t time_elapsed = clock() - start;

    if (!check_sol(g0, g1, &solution))
        fail("*** Error: Invalid solution\n");

    if (arguments.timeout == -1)
        printf("TIMEOUT\n");

    printf("Solution size %d\n", solution.len);
    printf("Number of edges in subgraph %d\n", count_edges(g0, g1, &solution));
    INSERTION_SORT(struct VtxPair, solution.vals, solution.len,
            (solution.vals[j-1].v > solution.vals[j].v))
    for (int i=0; i<solution.len; i++)
        printf("(%d -> %d) ", solution.vals[i].v, solution.vals[i].w);
    printf("\n");

    setlocale(LC_NUMERIC, "");
    printf("Nodes:                      %'15ld\n", stats.nodes);
    printf("CPU time (ms):              %15ld\n", time_elapsed * 1000 / CLOCKS_PER_SEC);

    free(preallocated_lists);
    free(g0);
    free(g1);
    free(g0_sorted);
    free(g1_sorted);
}

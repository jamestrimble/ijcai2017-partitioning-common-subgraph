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

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

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

static unsigned long long nodes = 0;

/*******************************************************************************
                                Graph functions
*******************************************************************************/

void calculate_all_degrees(struct Graph *g) {
    for (int v=0; v<g->n; v++) {
        g->degree[v] = 0;
        for (int w=0; w<g->n; w++) {
#ifdef LABELLED
            if  (g->adjmat[v][w]&0xFFFFu) g->degree[v]++;
            if  (g->adjmat[v][w]&0xFFFF0000u) g->degree[v]++; // inward edges, for directed case
#else
            if  (g->adjmat[v][w]&1) g->degree[v]++;
            if  (g->adjmat[v][w]&2) g->degree[v]++;   // inward edges, for directed case
#endif
        }
//        printf("%d\n", g->degree[v]);
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
    struct Bidomain *vals;
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

/*******************************************************************************
                              Preallocated structs
*******************************************************************************/

struct BidomainList *preallocated_lists;

// parameter n is the number of vertices in graph g0
void preallocate_lists(int n) {
    int num_lists = n + 1;
    int list_length = n;
    preallocated_lists = malloc(num_lists * sizeof(struct BidomainList));
    for (int i=0; i<num_lists; i++)
        preallocated_lists[i].vals = malloc(list_length * sizeof(struct Bidomain));
}

struct BidomainList *get_preallocated_list(int level) {
    struct BidomainList *list = &preallocated_lists[level];
    list->len = 0;
    return list;
}

void free_preallocated_lists(int n) {
    int num_lists = n + 1;
    for (int i=0; i<num_lists; i++)
        free(preallocated_lists[i].vals);
    free(preallocated_lists);
}

/******************************************************************************/

int calc_bound(struct BidomainList *domains) {
    int bound = 0;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        bound += MIN(bd->left_len, bd->right_len);
    }
    return bound;
}

int find_min(int *vv, int len) {
    int min = INT_MAX;
    for (int i=0; i<len; i++)
        if (vv[i] < min)
            min = vv[i];
    return min;
}

struct Bidomain *select_bidomain(struct BidomainList *domains, int current_matching_size) {
    // Select the bidomain with the smallest max(leftsize, rightsize), breaking
    // ties on the smallest vertex index in the left set
    struct Bidomain *bds[MAX_N];  // a list of bidomains of minimum size
    int bds_len = 0;
    int min_size = INT_MAX;
    for (int i=0; i<domains->len; i++) {
        struct Bidomain *bd = &domains->vals[i];
        if (arguments.connected && current_matching_size>0 && !bd->is_adjacent) continue;
        int size = MAX(bd->left_len, bd->right_len);
        if (size == min_size) {
            bds[bds_len++] = bd;
        } else if (size < min_size) {
            min_size = size;
            bds[0] = bd;
            bds_len = 1;
        }
    }
    // break ties
    int min_vtx_idx = INT_MAX;
    struct Bidomain *best = NULL;
    for (int i=0; i<bds_len; i++) {
        struct Bidomain *bd = bds[i];
        int vtx_idx = find_min(bd->left_vv, bd->left_len);
        if (vtx_idx < min_vtx_idx) {
            min_vtx_idx = vtx_idx;
            best = bd;
        }
    }
    return best;
}

// Returns length of left half of array
int partition(int *vv, int vv_len, edge_label_t *adjrow) {
    int i=0;
    for (int j=0; j<vv_len; j++) {
        if (adjrow[vv[j]]) {
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

int compare_by_edge_labels(const void *a, const void *b, void *ar)
{
    int v = *(int*) a;
    int w = *(int*) b;
    edge_label_t *adjrow = (edge_label_t *) ar;
    return (adjrow[v]>adjrow[w]) - (adjrow[v]<adjrow[w]);
}

// multiway must be true iff we are solving a labelled or directed problem
void filter_domains(struct BidomainList *d, struct BidomainList *new_d,
        struct Graph *g0, struct Graph *g1, int v, int w, bool multiway)
{
    new_d->len=0;
    for (int j=0; j<d->len; j++) {
        struct Bidomain *old_bd = &d->vals[j];
        int *l = old_bd->left_vv;
        int *r = old_bd->right_vv;
        // After these two partitions, left_len and right_len are the lengths of the
        // arrays of vertices with edges from v or w (int the directed case, edges
        // either from or to v or w)
        int left_len = partition(l, old_bd->left_len, g0->adjmat[v]);
        int right_len = partition(r, old_bd->right_len, g1->adjmat[w]);
        int left_len_noedge = old_bd->left_len - left_len;
        int right_len_noedge = old_bd->right_len - right_len;
        if (left_len_noedge && right_len_noedge) {
            add_bidomain(new_d, l+left_len, r+right_len,
                    left_len_noedge, right_len_noedge, old_bd->is_adjacent);
        }
        if (multiway && left_len && right_len) {
            edge_label_t *adjrow_v = g0->adjmat[v];
            edge_label_t *adjrow_w = g1->adjmat[w];
            qsort_r(l, left_len, sizeof(int), compare_by_edge_labels, adjrow_v);
            qsort_r(r, right_len, sizeof(int), compare_by_edge_labels, adjrow_w);
            int *left_top = l + left_len;
            int *right_top = r + right_len;
            while (l<left_top && r<right_top) {
                int left_label = adjrow_v[*l];
                int right_label = adjrow_w[*r];
                if (left_label < right_label) {
                    l++;
                } else if (left_label > right_label) {
                    r++;
                } else {  // The edges in the two graphs are equal
                    int *lmin = l;
                    int *rmin = r;
                    do { l++; } while (l<left_top && adjrow_v[*l]==left_label);
                    do { r++; } while (r<right_top && adjrow_w[*r]==left_label);
                    add_bidomain(new_d, lmin, rmin, l-lmin, r-rmin, true);
                }
            }
        } else if (left_len && right_len) {
            add_bidomain(new_d, l, r, left_len, right_len, true);
        }
    }
}

void remove_bidomain(struct BidomainList *list, struct Bidomain *b) {
    int i = b - &list->vals[0];
    list->vals[i] = list->vals[list->len-1];
    list->len--;
}

void show(struct D d) {
    printf("Nodes: %llu\n", nodes);
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
//    printf(" mcsp --- Nodes: %ld\n", stats.nodes);
    if (arguments.verbose) show(d);

    nodes++;
    if (arguments.timeout && nodes%100000==0 && 
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

    int v = find_and_remove_min_value(bd->left_vv, &bd->left_len);

    struct D new_d = d;
    new_d.domains = get_preallocated_list(level);

    // Try assigning v to each vertex w in bd->right_vv, in turn
    bd->right_len--;
    int w = -1;
    for (int i=0; i<=bd->right_len; i++) {
        int idx = index_of_next_smallest(bd->right_vv, bd->right_len+1, w);
        w = bd->right_vv[idx];
        
        // swap w with the value just past the end of the right_vv array
        bd->right_vv[idx] = bd->right_vv[bd->right_len];
        bd->right_vv[bd->right_len] = w;

#ifdef LABELLED
        filter_domains(d.domains, new_d.domains, d.g0, d.g1, v, w, true);
#else
        filter_domains(d.domains, new_d.domains, d.g0, d.g1, v, w, arguments.directed);
#endif
        d.current->vals[d.current->len++] = (struct VtxPair) {.v=v, .w=w};
        solve(new_d, level+1);
        d.current->len--;
    }
    bd->right_len++;
    if (bd->left_len == 0) {
        remove_bidomain(d.domains, bd);
    }
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
    struct BidomainList *domains = get_preallocated_list(0);

    int *left = malloc(g0->n * sizeof(*left));    // the buffer of vertex indices for the left partitions
    int *right = malloc(g1->n * sizeof(*right));  // the buffer of vertex indices for the right partitions
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

    free(left);
    free(right);
}

struct VtxPairList mcs(struct Graph *g0, struct Graph *g1) {
    unsigned int *all_labels = malloc((g0->n+g1->n) * sizeof(*all_labels));
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

    free(all_labels);
    return incumbent;
}

// Count edges in the subgraph
int count_edges(struct Graph *g0, struct Graph *g1, struct VtxPairList *solution) {
    int count = 0;
    for (int i=0; i<solution->len; i++) {
        struct VtxPair p0 = solution->vals[i];
        for (int j=i+1; j<solution->len; j++) {
            struct VtxPair p1 = solution->vals[j];
#ifdef LABELLED
            if (g0->adjmat[p0.v][p1.v]&0xFFFFu) count++;
            if (g0->adjmat[p0.v][p1.v]&0xFFFF0000u) count++;
#else
            if (g0->adjmat[p0.v][p1.v]&1) count++;
            if (g0->adjmat[p0.v][p1.v]&2) count++;
#endif
        }
    }
    return count;
}

bool check_sol(struct Graph *g0, struct Graph *g1, struct VtxPairList *solution) {
    bool *used_left = malloc(g0->n * sizeof(*used_left));
    bool *used_right = malloc(g1->n * sizeof(*used_right));
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
    free(used_left);
    free(used_right);
}

int graph_edge_count(struct Graph *g) {
    int count = 0;
    for (int i=0; i<g->n; i++)
        count += g->degree[i];
    return count;
}

int main(int argc, char** argv) {
    if (sizeof(unsigned int) < 4)
        fail("sizeof(unsigned int) must be at least 4.");

    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);

#ifdef LABELLED
    bool labelled = true;
#else
    bool labelled = false;
#endif

    enum graph_format format = arguments.dimacs ? DIMACS_FORMAT :
                               arguments.lad    ? LAD_FORMAT :
                                                  VF_FORMAT;
    struct Graph *g0 = read_graph(arguments.filename1, format, arguments.directed, labelled);
    struct Graph *g1 = read_graph(arguments.filename2, format, arguments.directed, labelled);

    start = clock();
    preallocate_lists(g0->n);
    calculate_all_degrees(g0);
    calculate_all_degrees(g1);

    int *vv0 = malloc(g0->n * sizeof(*vv0));
    for (int i=0; i<g0->n; i++) vv0[i] = i;
    if (graph_edge_count(g1) > g1->n*(g1->n-1)/2) {
        INSERTION_SORT(int, vv0, g0->n, (g0->degree[vv0[j-1]] > g0->degree[vv0[j]]))
    } else {
        INSERTION_SORT(int, vv0, g0->n, (g0->degree[vv0[j-1]] < g0->degree[vv0[j]]))
    }

    int *vv1 = malloc(g0->n * sizeof(*vv0));
    for (int i=0; i<g1->n; i++) vv1[i] = i;
    if (graph_edge_count(g0) > g0->n*(g0->n-1)/2) {
        INSERTION_SORT(int, vv1, g1->n, (g1->degree[vv1[j-1]] > g1->degree[vv1[j]]))
    } else {
        INSERTION_SORT(int, vv1, g1->n, (g1->degree[vv1[j-1]] < g1->degree[vv1[j]]))
    }

    struct Graph *g0_sorted = induced_subgraph(g0, vv0, g0->n);
    struct Graph *g1_sorted = induced_subgraph(g1, vv1, g1->n);

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
    printf("Nodes:                      %'15llu\n", nodes);
    printf("CPU time (ms):              %15ld\n", time_elapsed * 1000 / CLOCKS_PER_SEC);

    free(vv0);
    free(vv1);
    free_preallocated_lists(g0->n);
    free_graph(g0);
    free_graph(g1);
    free_graph(g0_sorted);
    free_graph(g1_sorted);
}

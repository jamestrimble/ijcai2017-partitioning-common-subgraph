#include "graph.h"

#include <argp.h>
#include <limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <chrono>

#include <cilk/cilk.h>

using std::vector;
using std::cout;
using std::endl;

static void fail(std::string msg) {
    std::cerr << msg << std::endl;
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
    VtxPair(int v, int w): v(v), w(w) {}
};

struct Bidomain {
    int l,        r;        // start indices of left and right sets
    int left_len, right_len;
    bool is_adjacent;
    Bidomain(int l, int r, int left_len, int right_len, bool is_adjacent):
            l(l),
            r(r),
            left_len (left_len),
            right_len (right_len),
            is_adjacent (is_adjacent) { };
};

bool check_sol(struct Graph *g0, struct Graph *g1, vector<VtxPair>& solution) {
    vector<bool> used_left(g0->n, false);
    vector<bool> used_right(g1->n, false);
    for (unsigned int i=0; i<solution.size(); i++) {
        struct VtxPair p0 = solution[i];
        if (used_left[p0.v] || used_right[p0.w])
            return false;
        used_left[p0.v] = true;
        used_right[p0.w] = true;
        if (g0->label[p0.v] != g1->label[p0.w])
            return false;
        for (unsigned int j=i+1; j<solution.size(); j++) {
            struct VtxPair p1 = solution[j];
            if (g0->adjmat[p0.v][p1.v] != g1->adjmat[p0.w][p1.w])
                return false;
        }
    }
    return true;
}

void show(const vector<VtxPair>& current, const vector<Bidomain> &domains,
        const vector<int>& left, const vector<int>& right)
{
    printf("Nodes: %ld\n", nodes);
    printf("Length of current assignment: %ld\n", current.size());
    printf("Current assignment:");
    for (unsigned int i=0; i<current.size(); i++) {
        printf("  %d->%d", current[i].v, current[i].w);
    }
    printf("\n");
    for (unsigned int i=0; i<domains.size(); i++) {
        struct Bidomain bd = domains[i];
        printf("Left  ");
        for (int j=0; j<bd.left_len; j++)
            printf("%d ", left[bd.l + j]);
        printf("\n");
        printf("Right  ");
        for (int j=0; j<bd.right_len; j++)
            printf("%d ", right[bd.r + j]);
        printf("\n");
    }
    printf("\n\n");
}

int calc_bound(const vector<Bidomain>& domains) {
    int bound = 0;
    for (const Bidomain &bd : domains) {
        bound += std::min(bd.left_len, bd.right_len);
    }
    return bound;
}

int select_bidomain(const vector<Bidomain>& domains, int current_matching_size) {
    // Select the bidomain with the smallest max(leftsize, rightsize), breaking
    // ties on the smallest vertex index in the left set
    int min_size = INT_MAX;
    int best = -1;
    for (unsigned int i=0; i<domains.size(); i++) {
    const Bidomain &bd = domains[i];
        if (arguments.connected && current_matching_size>0 && !bd.is_adjacent) continue;
        if (bd.left_len==0) continue;  // This could happen if a decision has been
                                       // made to reject a vertex
        int len = std::max(bd.left_len, bd.right_len);
        if (len < min_size) {
            min_size = len;
            best = i;
        }
    }
    return best;
}

void filter(const vector<int>& arr, int start_idx, int len,
        vector<int>&out_arr, int v, const std::vector<unsigned char>& adjrow)
{
    for (int j=0; j<len; j++) {
        int w = arr[start_idx + j];
        if (adjrow[w]) {
            out_arr.push_back(w);
        }
    }
}

vector<Bidomain> filter_domains(const vector<Bidomain> &d,
        const vector<int>& old_left, const vector<int>& old_right,
        vector<int>& new_left, vector<int>& new_right,
        struct Graph *g0, struct Graph *g1, int v, int w)
{
    vector<Bidomain> new_d;
    new_d.reserve(d.size());
    int l = 0;
    int r = 0;
    for (const Bidomain &old_bd : d) {
        if (!old_bd.left_len || !old_bd.right_len) continue;
        for (int edge=0; edge<=1; edge++) {
            filter(old_left, old_bd.l, old_bd.left_len, new_left, v,
                    edge ? g0->adjmat[v] : g0->antiadjmat[v]);
            int left_len = new_left.size() - l;
            if (left_len) {
                filter(old_right, old_bd.r, old_bd.right_len, new_right, w,
                        edge ? g1->adjmat[w] : g1->antiadjmat[w]);
                int right_len = new_right.size() - r;
                if (right_len) {
                    new_d.push_back({l, r, left_len, right_len, old_bd.is_adjacent | edge});
                    l = new_left.size();
                    r = new_right.size();
                } else {
                    new_left.resize(l);
                    new_right.resize(r);
                }
            } else {
                new_left.resize(l);
            }
        }
    }
    return new_d;
}

int find_min_value(const vector<int>& arr, int start_idx, int len) {
    int min_v = INT_MAX;
    for (int i=0; i<len; i++)
        if (arr[start_idx + i] < min_v)
            min_v = arr[start_idx + i];
    return min_v;
}

// returns the index of the smallest value in arr that is >w.
// Assumption: such a value exists
// Assumption: arr contains no duplicates
// Assumption: arr has no values==INT_MAX
int index_of_next_smallest(const vector<int>& arr, int start_idx, int len, int w) {
    int idx = -1;
    int smallest = INT_MAX;
    for (int i=0; i<len; i++) {
        if (arr[start_idx + i]>w && arr[start_idx + i]<smallest) {
            smallest = arr[start_idx + i];
            idx = i;
        }
    }
    return idx;
}

void remove_vtx_from_left_domain(vector<int>& left, Bidomain& bd, int v)
{
    int i = 0;
    while(left[bd.l + i] != v) i++;
    left[bd.l+i] = left[bd.l+bd.left_len-1];
    bd.left_len--;
}

void solve(struct Graph *g0, struct Graph *g1, vector<VtxPair>& incumbent,
        const vector<VtxPair>& current, const vector<Bidomain> &domains,
        const vector<int>& left, const vector<int>& right, int level)
{
    if (arguments.verbose) show(current, domains, left, right);
    nodes++;

    if (current.size() > incumbent.size()) {
        incumbent = current;
        if (!arguments.quiet) cout << "Incumbent size: " << incumbent.size() << endl;
    }

    if (current.size() + calc_bound(domains) <= incumbent.size())
        return;

    int bd_idx = select_bidomain(domains, current.size());
    if (bd_idx == -1)   // In the MCCS case, there may be nothing we can branch on
        return;
    const Bidomain &bd = domains[bd_idx];

    int v = find_min_value(left, bd.l, bd.left_len);

    // Try assigning v to each vertex w in bd.right_vv, in turn
    int w = -1;
    for (int i=0; i<bd.right_len; i++) {
        int idx = index_of_next_smallest(right, bd.r, bd.right_len, w);
        w = right[bd.r + idx];
        cilk_spawn [&left, &right, &bd, &domains, &g0, &g1, &current, &incumbent, v, w, level] {
            vector<int> new_left;
            new_left.reserve(left.size());
            vector<int> new_right;
            new_right.reserve(right.size());
            auto new_domains = filter_domains(
                    domains, left, right, new_left, new_right, g0, g1, v, w);
            auto new_current(current);
            new_current.push_back(VtxPair(v, w));
            solve(g0, g1, incumbent, new_current, new_domains, new_left, new_right, level+1);
        }();
    }
    auto new_domains = domains;
    auto new_left = left;
    remove_vtx_from_left_domain(new_left, new_domains[bd_idx], v);
    solve(g0, g1, incumbent, current, new_domains, new_left, right, level+1);

    cilk_sync;
}

vector<VtxPair> mcs(struct Graph *g0, struct Graph *g1) {
    vector<int> left;  // the buffer of vertex indices for the left partitions
    vector<int> right;  // the buffer of vertex indices for the right partitions

    auto domains = vector<Bidomain> {};

    // Create a bidomain for vertices without loops (label 0),
    // and another for vertices with loops (label 1)
    for (unsigned int label=0; label<=1; label++) {
        int start_l = left.size();
        int start_r = right.size();

        for (int i=0; i<g0->n; i++)
            if (g0->label[i]==label)
                left.push_back(i);
        for (int i=0; i<g1->n; i++)
            if (g1->label[i]==label)
                right.push_back(i);

        int left_len = left.size() - start_l;
        int right_len = left.size() - start_r;
        if (left_len && right_len)
            domains.push_back({start_l, start_r, left_len, right_len, false});
    }

    vector<VtxPair> incumbent, current;

    solve(g0, g1, incumbent, current, domains, left, right, 1);

    return incumbent;
}

int main(int argc, char** argv) {
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);

    char format = arguments.dimacs ? 'D' : arguments.lad ? 'L' : 'B';
    struct Graph g0 = readGraph(arguments.filename1, format);
    struct Graph g1 = readGraph(arguments.filename2, format);

    auto start = std::chrono::steady_clock::now();

    vector<VtxPair> solution = mcs(&g0, &g1);

    auto time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();

    if (!check_sol(&g0, &g1, solution))
        fail("*** Error: Invalid solution\n");

    printf("Solution size %ld\n", solution.size());
    for (int i=0; i<g0.n; i++)
        for (unsigned int j=0; j<solution.size(); j++)
            if (solution[j].v == i)
                printf("(%d -> %d) ", solution[j].v, solution[j].w);
    printf("\n");

    setlocale(LC_NUMERIC, "");
    printf("Nodes:                      %'15ld\n", nodes);
    printf("CPU time (ms):              %15ld\n", time_elapsed);
}

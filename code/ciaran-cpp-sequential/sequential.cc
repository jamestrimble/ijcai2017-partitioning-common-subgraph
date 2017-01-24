/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "sequential.hh"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <numeric>
#include <random>
#include <tuple>
#include <utility>
#include <vector>
#include <mutex>
#include <set>

#include <iostream>

using std::fill;
using std::iota;
using std::pair;
using std::vector;
using std::multimap;
using std::min;
using std::max;
using std::mutex;
using std::unique_lock;
using std::atomic;
using std::numeric_limits;
using std::make_pair;
using std::set;
using std::distance;

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;
using std::chrono::time_point;

using std::cerr;
using std::endl;

namespace
{
    struct Partition
    {
        multimap<int, int> colours_to_vertices;
    };

    struct Assignments
    {
        vector<pair<int, int> > mappings;
        unsigned mapped_size = 0;
    };

    struct Incumbent
    {
        atomic<unsigned> value{ 0 };

        mutex c_mutex;
        Assignments c;

        time_point<steady_clock> start_time;

        Result * result;

        void update(const Assignments & new_c)
        {
            while (true) {
                unsigned current_value = value;
                if (new_c.mapped_size > current_value) {
                    unsigned new_c_size = new_c.mapped_size;
                    if (value.compare_exchange_strong(current_value, new_c_size)) {
                        unique_lock<mutex> lock(c_mutex);

                        c = new_c;

                        result->isomorphism.clear();
                        for (auto & a : c.mappings)
                            result->isomorphism.emplace(a.first, a.second);

                        cerr << "-- " << (duration_cast<milliseconds>(steady_clock::now() - start_time)).count()
                            << " " << c.mapped_size;
                        for (auto & a : c.mappings)
                            cerr << " " << a.first << "=" << a.second;
                        cerr << endl;
                        break;
                    }
                }
                else
                    break;
            }
        }
    };

    struct MCS
    {
        const Params & params;
        const Graph & pattern;
        const Graph & target;
        Incumbent incumbent;
        Result result;

        MCS(const Params & k, const Graph & p, const Graph & t) :
            params(k),
            pattern(p),
            target(t)
        {
            incumbent.start_time = params.start_time;
            incumbent.result = &result;
        }

        auto calculate_bound(
                const Partition & pattern_partition,
                const Partition & target_partition) -> unsigned
        {
            if (pattern_partition.colours_to_vertices.empty())
                return 0;

            unsigned result = 0;
            for (int colour = 0 ; colour <= pattern_partition.colours_to_vertices.rbegin()->first ; ++colour) {
                result += min(pattern_partition.colours_to_vertices.count(colour), target_partition.colours_to_vertices.count(colour));
            }

            return result;
        }

        auto pick_branch_colour_and_vertex(
                const Partition & pattern_partition,
                const Partition & target_partition) -> pair<int, int>
        {
            int branch_colour = -1, size = numeric_limits<int>::max(), bsize = numeric_limits<int>::max();

            for (int colour = 0 ; colour <= pattern_partition.colours_to_vertices.rbegin()->first ; ++colour) {
                int count = pattern_partition.colours_to_vertices.count(colour);
                int bcount = target_partition.colours_to_vertices.count(colour);
                if (count > 0 && (count < size || (count == size && bcount < bsize))) {
                    branch_colour = colour;
                    size = count;
                    bsize = bcount;
                }
            }

            int branch_vertex = -1;
            for (auto range = pattern_partition.colours_to_vertices.equal_range(branch_colour) ; range.first != range.second ; ++range.first) {
                if (-1 == branch_vertex)
                    branch_vertex = range.first->second;
            }

            return make_pair(branch_colour, branch_vertex);
        }

        auto repartition(
                const int pattern_vertex,
                const int target_vertex,
                Partition & pattern_partition,
                Partition & target_partition) -> void
        {
            auto p = move(pattern_partition.colours_to_vertices);
            auto t = move(target_partition.colours_to_vertices);

            set<int> used_colours;

            pattern_partition.colours_to_vertices.clear();

            for (auto & c_v : p) {
                if (c_v.second == pattern_vertex)
                    continue;

                if (pattern.adjacent(pattern_vertex, c_v.second)) {
                    pattern_partition.colours_to_vertices.emplace(c_v.first * 2, c_v.second);
                    used_colours.insert(c_v.first * 2);
                }
                else {
                    used_colours.insert(c_v.first * 2 + 1);
                    pattern_partition.colours_to_vertices.emplace(c_v.first * 2 + 1, c_v.second);
                }
            }

            target_partition.colours_to_vertices.clear();

            for (auto & c_v : t) {
                if (c_v.second == target_vertex)
                    continue;

                if (target.adjacent(target_vertex, c_v.second)) {
                    target_partition.colours_to_vertices.emplace(c_v.first * 2, c_v.second);
                    used_colours.insert(c_v.first * 2);
                }
                else {
                    target_partition.colours_to_vertices.emplace(c_v.first * 2 + 1, c_v.second);
                    used_colours.insert(c_v.first * 2 + 1);
                }
            }

            p = move(pattern_partition.colours_to_vertices);
            t = move(target_partition.colours_to_vertices);

            for (auto & c_v : p)
                pattern_partition.colours_to_vertices.emplace(distance(used_colours.begin(), used_colours.find(c_v.first)), c_v.second);

            for (auto & c_v : t)
                target_partition.colours_to_vertices.emplace(distance(used_colours.begin(), used_colours.find(c_v.first)), c_v.second);
        }

        auto solve(
                int depth,
                Assignments & assignments,
                const Partition & pattern_partition,
                const Partition & target_partition) -> void
        {
            if (*params.abort)
                return;

            ++result.nodes;

            int bound = calculate_bound(pattern_partition, target_partition);

            if (0 == bound || assignments.mapped_size + bound <= incumbent.value)
                return;

            auto branch_colour_and_vertex = pick_branch_colour_and_vertex(pattern_partition, target_partition);

            if (-1 == branch_colour_and_vertex.first)
                return;

            for (auto allowed_mappings = target_partition.colours_to_vertices.equal_range(branch_colour_and_vertex.first) ;
                    allowed_mappings.first != allowed_mappings.second ; ++allowed_mappings.first) {
                auto map_to_target_vertex = allowed_mappings.first->second;

                assignments.mappings.emplace_back(branch_colour_and_vertex.second, map_to_target_vertex);
                ++assignments.mapped_size;
                incumbent.update(assignments);

                auto new_pattern_partition = pattern_partition;
                auto new_target_partition = target_partition;

                repartition(branch_colour_and_vertex.second, map_to_target_vertex, new_pattern_partition, new_target_partition);

                solve(depth + 1, assignments, new_pattern_partition, new_target_partition);

                assignments.mappings.pop_back();
                --assignments.mapped_size;
            }

            auto reduced_pattern_partition = pattern_partition;
            auto range = reduced_pattern_partition.colours_to_vertices.equal_range(branch_colour_and_vertex.first);
            for ( ; range.first != range.second ; ++range.first)
                if (range.first->second == branch_colour_and_vertex.second) {
                    reduced_pattern_partition.colours_to_vertices.erase(range.first);
                    break;
                }

            solve(depth + 1, assignments, reduced_pattern_partition, target_partition);
        }

        auto initialise_partition(const Graph & g, Partition & partition) -> void
        {
            for (unsigned i = 0 ; i < g.size() ; ++i)
                partition.colours_to_vertices.emplace(g.adjacent(i, i) ? 1 : 0, i);
        }

        auto run()
        {
            Partition pattern_partition, target_partition;
            initialise_partition(pattern, pattern_partition);
            initialise_partition(target, target_partition);

            Assignments assignments;
            solve(0, assignments, pattern_partition, target_partition);

            for (unsigned i = 0 ; i < pattern.size() ; ++i)
                result.isomorphism.emplace(i, -1);
        }
    };
}

auto sequential_max_common_subgraph(const pair<Graph, Graph> & graphs, const Params & params) -> Result
{
    MCS mcs(params, graphs.first, graphs.second);
    mcs.run();
    return mcs.result;
}


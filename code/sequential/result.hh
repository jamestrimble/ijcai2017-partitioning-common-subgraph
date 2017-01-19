/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CODE_GUARD_RESULT_HH
#define CODE_GUARD_RESULT_HH 1

#include <map>
#include <list>
#include <chrono>
#include <string>

struct Result
{
    /// The isomorphism, empty if none found.
    std::map<int, int> isomorphism;

    /// Total number of nodes processed.
    unsigned long long nodes = 0;

    /**
     * Runtimes. The first entry in the list is the total runtime.
     * Additional values are for each worker thread.
     */
    std::list<std::chrono::milliseconds> times;

    std::map<std::string, std::string> stats;
};

#endif

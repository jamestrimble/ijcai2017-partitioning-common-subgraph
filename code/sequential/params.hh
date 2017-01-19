/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CODE_GUARD_PARAMS_HH
#define CODE_GUARD_PARAMS_HH 1

#include <chrono>
#include <atomic>

struct Params
{
    /// If this is set to true, we should abort due to a time limit.
    std::atomic<bool> * abort;

    /// The start time of the algorithm.
    std::chrono::time_point<std::chrono::steady_clock> start_time;
};

#endif

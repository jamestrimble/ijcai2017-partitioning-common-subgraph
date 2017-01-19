/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_VF_HH
#define PARASOLS_GUARD_GRAPH_VF_HH 1

#include "graph.hh"
#include <string>

/**
 * Read a VF format file into a Graph.
 */
auto read_vf(const std::string & filename) -> Graph;

#endif

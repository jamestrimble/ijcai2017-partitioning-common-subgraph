/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_DIMACS_HH
#define PARASOLS_GUARD_GRAPH_DIMACS_HH 1

#include "graph.hh"
#include <string>

/**
 * Read a DIMACS format file into a Graph.
 */
auto read_dimacs(const std::string & filename) -> Graph;

#endif

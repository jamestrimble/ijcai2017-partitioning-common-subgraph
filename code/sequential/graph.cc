/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "graph.hh"
#include <algorithm>

GraphFileError::GraphFileError(const std::string & filename, const std::string & message) throw () :
    _what("Error reading graph file '" + filename + "': " + message)
{
}

auto GraphFileError::what() const throw () -> const char *
{
    return _what.c_str();
}

Graph::Graph(unsigned size)
{
    if (0 != size)
        resize(size);
}

auto Graph::_position(unsigned a, unsigned b) const -> AdjacencyMatrix::size_type
{
    return (a * _size) + b;
}

auto Graph::resize(unsigned size) -> void
{
    _size = size;
    _adjacency.resize(size * size);
}

auto Graph::add_edge(unsigned a, unsigned b) -> void
{
    _adjacency[_position(a, b)] = 1;
    _adjacency[_position(b, a)] = 1;
}

auto Graph::adjacent(unsigned a, unsigned b) const -> bool
{
    return _adjacency[_position(a, b)];
}

auto Graph::size() const -> unsigned
{
    return _size;
}

auto Graph::degree(unsigned a) const -> unsigned
{
    return std::count_if(
            _adjacency.begin() + a * _size,
            _adjacency.begin() + (a + 1) * _size,
            [] (unsigned x) { return !!x; });
}


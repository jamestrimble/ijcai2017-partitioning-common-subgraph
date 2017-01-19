/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GUARD_GRAPH_HH
#define GUARD_GRAPH_HH 1

#include <vector>
#include <string>
#include <type_traits>
#include <cstdint>

/**
 * Thrown if we come across bad data in a graph file, or if we can't read a
 * graph file.
 */
class GraphFileError :
    public std::exception
{
    private:
        std::string _what;

    public:
        GraphFileError(const std::string & filename, const std::string & message) throw ();

        auto what() const throw () -> const char *;
};

/**
 * A graph, with an adjacency matrix representation. We only provide the
 * operations we actually need.
 *
 * Indices start at 0.
 */
class Graph
{
    public:
        /**
         * The adjaceny matrix type. Shouldn't really be public, but we
         * snoop around inside it when doing message passing.
         */
        using AdjacencyMatrix = std::vector<std::uint8_t>;

    private:
        unsigned _size = 0;
        AdjacencyMatrix _adjacency;
        bool _add_one_for_output;

        /**
         * Return the appropriate offset into _adjacency for the edge (a,
         * b).
         */
        auto _position(unsigned a, unsigned b) const -> AdjacencyMatrix::size_type;

    public:
        /**
         * \param initial_size can be 0, if resize() is called afterwards.
         */
        Graph(unsigned initial_size);

        Graph(const Graph &) = default;

        explicit Graph() = default;

        /**
         * Number of vertices.
         */
        auto size() const -> unsigned;

        /**
         * Change our size. Must be called before adding an edge, and must
         * not be called afterwards.
         */
        auto resize(unsigned size) -> void;

        /**
         * Add an edge from a to b (and from b to a).
         */
        auto add_edge(unsigned a, unsigned b) -> void;

        /**
         * Are vertices a and b adjacent?
         */
        auto adjacent(unsigned a, unsigned b) const -> bool;

        /**
         * What is the degree of a given vertex?
         */
        auto degree(unsigned a) const -> unsigned;
};

#endif

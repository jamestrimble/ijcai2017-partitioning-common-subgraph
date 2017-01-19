/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "lad.hh"
#include "graph.hh"

#include <fstream>

namespace
{
    auto read_word(std::ifstream & infile) -> unsigned
    {
        unsigned x;
        infile >> x;
        return x;
    }
}

auto read_lad(const std::string & filename) -> Graph
{
    Graph result(0);

    std::ifstream infile{ filename };
    if (! infile)
        throw GraphFileError{ filename, "unable to open file" };

    result.resize(read_word(infile));
    if (! infile)
        throw GraphFileError{ filename, "error reading size" };

    for (unsigned r = 0 ; r < result.size() ; ++r) {
        unsigned c_end = read_word(infile);
        if (! infile)
            throw GraphFileError{ filename, "error reading edges count" };

        for (unsigned c = 0 ; c < c_end ; ++c) {
            unsigned e = read_word(infile);

            if (e >= result.size())
                throw GraphFileError{ filename, "edge index out of bounds" };

            result.add_edge(r, e);
        }
    }

    std::string rest;
    if (infile >> rest)
        throw GraphFileError{ filename, "EOF not reached, next text is \"" + rest + "\"" };
    if (! infile.eof())
        throw GraphFileError{ filename, "EOF not reached" };

    return result;
}


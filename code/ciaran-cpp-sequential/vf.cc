/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "vf.hh"
#include "graph.hh"
#include <boost/regex.hpp>
#include <fstream>

namespace
    {
    auto read_word(std::ifstream & infile) -> unsigned
    {
        unsigned char a, b;
        a = static_cast<unsigned char>(infile.get());
        b = static_cast<unsigned char>(infile.get());
        return unsigned(a) | (unsigned(b) << 8);
    }
}

auto read_vf(const std::string & filename) -> Graph
{
    Graph result;

    std::ifstream infile{ filename };
    if (! infile)
        throw GraphFileError{ filename, "unable to open file" };

    result.resize(read_word(infile));
    if (! infile)
        throw GraphFileError{ filename, "error reading size" };

    for (unsigned r = 0 ; r < result.size() ; ++r) {
        read_word(infile);
    }

    if (! infile)
        throw GraphFileError{ filename, "error reading attributes" };

    for (unsigned r = 0 ; r < result.size() ; ++r) {
        int c_end = read_word(infile);
        if (! infile)
            throw GraphFileError{ filename, "error reading edges count" };

        for (int c = 0 ; c < c_end ; ++c) {
            unsigned e = read_word(infile);

            if (e >= result.size())
                throw GraphFileError{ filename, "edge index " + std::to_string(e) + " out of bounds" };

            result.add_edge(r, e);
            read_word(infile);
        }
    }

    infile.peek();
    if (! infile.eof())
        throw GraphFileError{ filename, "EOF not reached" };

    return result;
}


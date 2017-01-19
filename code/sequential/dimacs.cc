/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "dimacs.hh"
#include "graph.hh"
#include <boost/regex.hpp>
#include <fstream>

auto read_dimacs(const std::string & filename) -> Graph
{
    Graph result;

    std::ifstream infile{ filename };
    if (! infile)
        throw GraphFileError{ filename, "unable to open file" };

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty())
            continue;

        /* Lines are comments, a problem description (contains the number of
         * vertices), or an edge. */
        static const boost::regex
            comment{ R"(c(\s.*)?)" },
            problem{ R"(p\s+(edge|col)\s+(\d+)\s+(\d+)?\s*)" },
            edge{ R"(e\s+(\d+)\s+(\d+)\s*)" };

        boost::smatch match;
        if (regex_match(line, match, comment)) {
            /* Comment, ignore */
        }
        else if (regex_match(line, match, problem)) {
            /* Problem. Specifies the size of the graph. Must happen exactly
             * once. */
            if (0 != result.size())
                throw GraphFileError{ filename, "multiple 'p' lines encountered" };
            result.resize(std::stoi(match.str(2)));
        }
        else if (regex_match(line, match, edge)) {
            /* An edge. DIMACS files are 1-indexed. We assume we've already had
             * a problem line (if not our size will be 0, so we'll throw). */
            int a{ std::stoi(match.str(1)) }, b{ std::stoi(match.str(2)) };
            if (0 == a || 0 == b || a > int(result.size()) || b > int(result.size()))
                throw GraphFileError{ filename, "line '" + line + "' edge index out of bounds" };
            else if (a == b)
                throw GraphFileError{ filename, "line '" + line + "' contains a loop" };
            result.add_edge(a - 1, b - 1);
        }
        else
            throw GraphFileError{ filename, "cannot parse line '" + line + "'" };
    }

    if (! infile.eof())
        throw GraphFileError{ filename, "error reading file" };

    return result;
}


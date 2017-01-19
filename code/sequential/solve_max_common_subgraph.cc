/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "lad.hh"
#include "vf.hh"
#include "dimacs.hh"
#include "sequential.hh"

#include <boost/program_options.hpp>

#include <iostream>
#include <exception>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace po = boost::program_options;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

/* Helper: return a function that runs the specified algorithm, dealing
 * with timing information and timeouts. */
template <typename Result_, typename Params_, typename Data_>
auto run_this_wrapped(const std::function<Result_ (const Data_ &, const Params_ &)> & func)
    -> std::function<Result_ (const Data_ &, Params_ &, bool &, int)>
{
    return [func] (const Data_ & data, Params_ & params, bool & aborted, int timeout) -> Result_ {
        /* For a timeout, we use a thread and a timed CV. We also wake the
         * CV up if we're done, so the timeout thread can terminate. */
        std::thread timeout_thread;
        std::mutex timeout_mutex;
        std::condition_variable timeout_cv;
        std::atomic<bool> abort;
        abort.store(false);
        params.abort = &abort;
        if (0 != timeout) {
            timeout_thread = std::thread([&] {
                    auto abort_time = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
                    {
                        /* Sleep until either we've reached the time limit,
                         * or we've finished all the work. */
                        std::unique_lock<std::mutex> guard(timeout_mutex);
                        while (! abort.load()) {
                            if (std::cv_status::timeout == timeout_cv.wait_until(guard, abort_time)) {
                                /* We've woken up, and it's due to a timeout. */
                                aborted = true;
                                break;
                            }
                        }
                    }
                    abort.store(true);
                    });
        }

        /* Start the clock */
        params.start_time = std::chrono::steady_clock::now();

        try {
            auto result = func(data, params);

            /* Clean up the timeout thread */
            if (timeout_thread.joinable()) {
                {
                    std::unique_lock<std::mutex> guard(timeout_mutex);
                    abort.store(true);
                    timeout_cv.notify_all();
                }
                timeout_thread.join();
            }

            return result;
        }
        catch (...) {
            /* Clean up the timeout thread */
            if (timeout_thread.joinable()) {
                {
                    std::unique_lock<std::mutex> guard(timeout_mutex);
                    abort.store(true);
                    timeout_cv.notify_all();
                }
                timeout_thread.join();
            }

            throw;
        }
    };
}

/* Helper: return a function that runs the specified algorithm, dealing
 * with timing information and timeouts. */
template <typename Result_, typename Params_, typename Data_>
auto run_this(Result_ func(const Data_ &, const Params_ &)) -> std::function<Result_ (const Data_ &, Params_ &, bool &, int)>
{
    return run_this_wrapped(std::function<Result_ (const Data_ &, const Params_ &)>(func));
}

auto main(int argc, char * argv[]) -> int
{
    auto mcs_algorithms = {
        std::make_pair( std::string{ "sequential" },         sequential_max_common_subgraph )
    };

    try {
        po::options_description display_options{ "Program options" };
        display_options.add_options()
            ("help",                                  "Display help information")
            ("timeout",            po::value<int>(),  "Abort after this many seconds")
            ("format",             po::value<std::string>(), "Specify graph file format (lad, dimacs or vf)")
            ;

        po::options_description all_options{ "All options" };
        all_options.add_options()
            ("algorithm",    "Specify which algorithm to use")
            ("pattern-file", "Specify the pattern file (LAD format)")
            ("target-file",  "Specify the target file (LAD format)")
            ;

        all_options.add(display_options);

        po::positional_options_description positional_options;
        positional_options
            .add("algorithm", 1)
            .add("pattern-file", 1)
            .add("target-file", 1)
            ;

        po::variables_map options_vars;
        po::store(po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(positional_options)
                .run(), options_vars);
        po::notify(options_vars);

        /* --help? Show a message, and exit. */
        if (options_vars.count("help")) {
            std::cout << "Usage: " << argv[0] << " [options] algorithm pattern target" << std::endl;
            std::cout << std::endl;
            std::cout << display_options << std::endl;
            return EXIT_SUCCESS;
        }

        /* No algorithm or no input file specified? Show a message and exit. */
        if (! options_vars.count("algorithm") || ! options_vars.count("pattern-file") || ! options_vars.count("target-file")) {
            std::cout << "Usage: " << argv[0] << " [options] algorithm pattern target" << std::endl;
            return EXIT_FAILURE;
        }

        /* Turn an algorithm string name into a runnable function. */
        auto algorithm = mcs_algorithms.begin(), algorithm_end = mcs_algorithms.end();
        for ( ; algorithm != algorithm_end ; ++algorithm)
            if (algorithm->first == options_vars["algorithm"].as<std::string>())
                break;

        /* Unknown algorithm? Show a message and exit. */
        if (algorithm == algorithm_end) {
            std::cerr << "Unknown algorithm " << options_vars["algorithm"].as<std::string>() << ", choose from:";
            for (auto a : mcs_algorithms)
                std::cerr << " " << a.first;
            std::cerr << std::endl;
            return EXIT_FAILURE;
        }

        /* Figure out what our options should be. */
        Params params;

        auto read_function = read_lad;

        if (options_vars.count("format")) {
            if (options_vars["format"].as<std::string>() == "lad")
                read_function = read_lad;
            else if (options_vars["format"].as<std::string>() == "dimacs")
                read_function = read_dimacs;
            else if (options_vars["format"].as<std::string>() == "vf")
                read_function = read_vf;
            else {
                std::cerr << "Unknown format " << options_vars["format"].as<std::string>() << std::endl;
                return EXIT_FAILURE;
            }
        }

        /* Read in the graphs */
        auto graphs = std::make_pair(
            read_function(options_vars["pattern-file"].as<std::string>()),
            read_function(options_vars["target-file"].as<std::string>()));

        /* Do the actual run. */
        bool aborted = false;
        auto result = run_this(algorithm->second)(
                graphs,
                params,
                aborted,
                options_vars.count("timeout") ? options_vars["timeout"].as<int>() : 0);

        /* Stop the clock. */
        auto overall_time = duration_cast<milliseconds>(steady_clock::now() - params.start_time);

        /* Display the results. */
        std::cout << std::boolalpha << ! result.isomorphism.empty() << " " << result.nodes;

        unsigned hit = 0;
        for (auto v : result.isomorphism)
            if (v.second != -1)
                ++hit;

        std::cout << " " << hit;

        if (aborted)
            std::cout << " aborted";
        std::cout << std::endl;

        for (auto v : result.isomorphism)
            std::cout << "(" << v.first << " -> " << v.second << ") ";
        std::cout << std::endl;

        std::cout << overall_time.count();
        if (! result.times.empty()) {
            for (auto t : result.times)
                std::cout << " " << t.count();
        }
        std::cout << std::endl;

        if (! result.stats.empty()) {
            for (auto & s : result.stats) {
                std::cout << s.first << "=" << s.second << " ";
            }
            std::cout << std::endl;
        }

        if (! result.isomorphism.empty()) {
            for (unsigned i = 0 ; i < graphs.first.size() ; ++i) {
                for (unsigned j = 0 ; j < graphs.first.size() ; ++j) {
                    if (graphs.first.adjacent(i, j)) {
                        if (result.isomorphism.find(i)->second != -1 && result.isomorphism.find(j)->second != -1) {
                            if (! graphs.second.adjacent(result.isomorphism.find(i)->second, result.isomorphism.find(j)->second)) {
                                std::cerr << "Oops! not an isomorphism: " << i << " -- " << j << " but "
                                   << result.isomorphism.find(i)->second << " -/- " << result.isomorphism.find(j)->second << std::endl;
                                return EXIT_FAILURE;
                            }
                        }
                    }
                    else if (! graphs.first.adjacent(i, j)) {
                        if (result.isomorphism.find(i)->second != -1 && result.isomorphism.find(j)->second != -1) {
                            if (graphs.second.adjacent(result.isomorphism.find(i)->second, result.isomorphism.find(j)->second)) {
                                std::cerr << "Oops! not an induced isomorphism: " << i << " -/- " << j << " but "
                                   << result.isomorphism.find(i)->second << " -- " << result.isomorphism.find(j)->second << std::endl;
                                return EXIT_FAILURE;
                            }
                        }
                    }
                }
            }
        }

        return EXIT_SUCCESS;
    }
    catch (const po::error & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Try " << argv[0] << " --help" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}


#pragma once

#include <fmt/format.h>
#include <fmt/compile.h>

#include "ioh/problem/problem.hpp"
#include "ioh/experiment/configuration.hpp"

namespace ioh::logger
{
    /** A logger that stores some information in a single, tabular-like, file.
     * 
     * Each line displays the problem metadata and the watched properties.
     * This format displays a lot of redundant information, but is very easy to parse.
     * 
     * @code
        logger::FlatFile(
            {trigger::always},
            {watch::evaluations, watch::transformed_y},
            "my_experiment.dat",
            "./today/"
        );
     * @endcode
     *
     * @ingroup Loggers
     */
    class FlatFile : public Watcher {
    protected:
            const std::string _sep;
            const std::string _com;
            const std::string _eol;
            const std::string _nan;
            const std::array<const std::string,7> _heads;
            const bool _repeat_header;
            bool _has_header;

            const fs::path _expe_dir;
            const std::string _filename;
            
            std::ofstream _out;

            std::string _current_suite;
            size_t _current_run;

    public:
        /** The logger should at least track one logger::Property, or else it makes no sense to use it.
         *
         * @param triggers When to fire a log event.
         * @param properties What to log.
         * @param filename File name in which to write the logged properties.
         * @param output_directory Directory in which to put the data file.
         * @param separator The string separating fields.
         * @param comment The string indicating a comment.
         * @param no_value The string indicating that a watched property does not exists in this context.
         * @param end_of_line The string to use when all fields have been written.
         * @param common_header_titles Seven strings to print in the header for the common problem meta data (property names are automatically printed after).
         * @param repeat_header If true, the commented header is printed for each new run.
         */
        FlatFile(
              std::vector<std::reference_wrapper<logger::Trigger >> triggers,
              std::vector<std::reference_wrapper<logger::Property>> properties,
              const std::string filename = "IOH.dat",
              const fs::path output_directory = fs::current_path(),
              const std::string separator = "\t",
              const std::string comment = "# ",
              const std::string no_value = "None",
              const std::string end_of_line = "\n",
              const std::array<const std::string,7> common_header_titles
                  = {"suite_name", "problem_name", "problem_id", "problem_instance", "optimization_type", "dimension","run"},
              const bool repeat_header = false
        )
        : Watcher(triggers, properties)
        , _sep(separator)
        , _com(comment)
        , _eol(end_of_line)
        , _nan(no_value)
        , _heads(common_header_titles)
        , _repeat_header(repeat_header)
        , _has_header(false)
        , _expe_dir(output_directory)
        , _filename(filename)
        , _current_suite("unknown_suite")
        , _current_run(0)
        {
            // Data file
            if(not exists(_expe_dir)) {
                IOH_DBG(debug,"some directories do not exist in " << _expe_dir << ", try to create them");
                create_directories(_expe_dir);
            }
            IOH_DBG(debug,"will output data in " << _expe_dir/_filename);
            _out = std::ofstream(_expe_dir/_filename);
        }


        virtual void attach_suite(const std::string& suite_name)  override
        {
            _current_suite = suite_name;
        }

        virtual void attach_problem(const problem::MetaData& problem) override
        {
            // If this is a new problem.
            if(_problem == nullptr or *_problem != problem) {
                IOH_DBG(xdebug,"reset run counter");
                _current_run = 0; // Then reset the run counter.
            } else {
                IOH_DBG(xdebug,"increment run counter");
                _current_run++; // Then it's a new run.
            }

            Logger::attach_problem(problem); // update _problem

            if(_repeat_header or not _has_header) {
                IOH_DBG(xdebug,"print header");
                _out << _com;
                for(const std::string head : _heads) {
                    _out << _sep << head;
                }
                for(const auto& [name,p_ref] : _properties) {
                    _out << _sep << name;
                }
                _out << _eol;
                _out.flush();
                _has_header = true;
            }
        }

        virtual void call(const logger::Info& log_info) override
        {
            // Common static values
            // FIXME this should be cached as a string in attach_problem.
            IOH_DBG(xdebug,"print problem meta data");
            _out /* no sep */ << _current_suite;
            _out  <<   _sep   << _problem->name;
            _out  <<   _sep   << _problem->problem_id;
            _out  <<   _sep   << _problem->instance;
            _out  <<   _sep   << (_problem->optimization_type == common::OptimizationType::Minimization ? "min" : "max");
            _out  <<   _sep   << _problem->n_variables;
            _out  <<   _sep   << _current_run;

            IOH_DBG(xdebug,"print watched properties");
            for(const auto& [name, p_ref] : _properties) {
                auto property = p_ref.get()(log_info);
                if(property) {
                    _out << _sep << fmt::format(FMT_COMPILE("{:f}"), property.value());
                } else {
                    _out << _sep << _nan;
                }
            }
            _out << _eol;
            _out.flush();
        }

        ~FlatFile()
        {
            IOH_DBG(debug, "close data file");
            _out.close();
        }

    };
    

}

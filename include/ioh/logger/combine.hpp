#pragma once

#include <cassert>

#include "loggers.hpp"


namespace ioh::logger
{
    /** An logger that can hold several loggers and call them all.
     *
     * Useful if one want to use several loggers without having to manage them
     * separately.
     *
     * Example:
     * @code FIXME update this example
            BBOB_suite bench({1},{2},{3});
            using Input = BBOB_suite::Input_type;
    
            ecdf_logger<Input> log_ecdf(0,6e7,20, 0,100,20);
            csv_logger<Input> log_csv;
    
            // Combine them all
            LoggerCombine<Input> loggers(log_ecdf);
            loggers.add(log_csv);
    
            // Now you can single call.
            loggers.track_suite(bench);
            // etc.
     * @endcode
     *
     * @note: Loggers are guaranteed to be called in the order they are added.
     *
     * @ingroup Loggers
     */
    class Combine final : public Logger
    {
    protected:
        //! Store the managed loggers.
        std::vector<std::reference_wrapper<Logger>> _loggers;

    public:

        /** Takes at least one mandatory logger,
         * because an empty instance would be illogical.
         */
        explicit Combine(Logger& logger)
         : Logger()
         , _loggers(1, logger)
        { }

        /** Handle several loggers at once, but you have to pass pointers.
         *
         * @note you can use initializer lists to instantiate the given std::vector:
         * 
         * @code
            LoggerCombine loggers({log_ecdf, log_csv});
         * @endcode
         */
        explicit Combine(std::vector<std::reference_wrapper<Logger>> loggers)
        : Logger()
        , _loggers(std::move(loggers))
        {
            assert(not _loggers.empty());
        }

        /** Add another logger to the list.
         */
        void append(Logger& logger)
        {
            _loggers.push_back(logger);
        }

        /** Logger interface */
        /** @{ */
        void attach_suite(const std::string& suite_name) override
        {
            for(auto& logger : _loggers) {
                logger.get().attach_suite(suite_name);
            }
        }

        void attach_problem(const problem::MetaData& problem) override
        {
            Logger::attach_problem(problem);
            for(auto& logger : _loggers) {
                logger.get().attach_problem(problem);
            }
        }

        // We override the high-level interface because a Combine has no properties or triggers.
        // Hence, we cannot use the Logger's implementation, which check for existing ones.
        // So here, we just proxy to sub-loggers' `log` method to do it.
        void log(const logger::Info &logger_info) override
        {
            IOH_DBG(debug,"call sub-loggers");
            for(auto &logger : _loggers) {
                logger.get().log(logger_info);
            }
        }

        // Given that we override `log`, this should never be called.
        void call(const logger::Info &/*logger_info*/) override
        {
            IOH_DBG(error,"this interface should not be called from here");
            throw std::runtime_error("logger::Combine::call should not be called directly.");
        }

        void reset() override
        {
            Logger::reset();
            for(auto& logger : _loggers) {
                logger.get().reset();
            }
        }

        /** @} */
    };
}

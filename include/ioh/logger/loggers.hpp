#pragma once

#include <stdexcept>

#include "../problem/utils.hpp"

#include "loginfo.hpp"
#include "triggers.hpp"
#include "properties.hpp"

namespace ioh {


    // This is out of the logger namespace, to avoid name repetition.
    /** Interface of loggers.
     * 
     * A logger is a class that has a `log(log_info)` function called by the problem
     * and hosts a set of Trigger that determine when a log event should be handled (or silently passed).
     * 
     * @warning Using the call functions without having been attached to a problem first will results
     *          in an assert failure.
     * 
     * By default, the Logger calls a log event if any of the triggers are fired,
     * but you can use your own way to combine triggers if necessary.
     * 
     * If you need to handle any property, consider sub-classing logger::Watcher.
     * 
     * If not, the design enforces your subclass to declare which property it needs.
     * Then, you can easily access them by their name through `_properties.at("name")`.
     * 
     * @ingroup Logging
     */
    class Logger {
    protected:
        //! Default combination of triggers instance.
        trigger::Any _any;
        
        //! Lower-level interface.
        trigger::Set& _triggers;

        //! Access to the problem.
        const problem::MetaData* _problem;

        /** Map property names to property references.
         * 
         * If your logger is handling any property, you can just iterate over them,
         * if you need specific ones, you can call them by their name.
         * 
         * This holds reference to optionals, so to get the actual value of a property, you should:
         *   -# use `at(…)` and not `operator[]`,
         *   -# use `get()` on the returned element,
         *   -# ensure that there actually is a valid value,
         *   -# use `value()` to get it.
         * @code
         * // With its name:
         * _properties.at("name").get().value()
         * // If your iterating over the map:
         * for(auto& p : _properties) { p.second.get().value();}
         * @endcode
         * 
         * You can directly test if the property actually has a valid value:
         * @code
         * if(_properties.at("name").get()) {...}
         * @endcode
         */
        // We need references because we have container of instances of various classes.
        // As all share a common interface, we can have a container of references of this base class.
        std::map<std::string,std::reference_wrapper<logger::Property>> _properties;

#ifndef NDEBUG
        //! Check that there is no duplicated properties (only in Debug builds).
        bool consistent_properties()
        {
            if(_properties.size() <= 0) {return false;}

            for(auto iw = std::begin(_properties); iw != std::end(_properties); iw++) {
                auto inext = iw; inext++;
                if(inext == std::end(_properties)) { break; }
                // Search only in toward the end, to avoid useless double checks.
                if(std::find_if(inext,std::end(_properties),
                                [iw](const auto& rwp){return iw->second.get().name() == rwp.second.get().name();}
                               ) != std::end(_properties)) {
                    return false;
                }
            }
            return true;
        }
#endif

        //! Convert a vector of properties in a map of {name => property}.
        void map_properties(std::vector<std::reference_wrapper<logger::Property>>& properties)
        {
            for(auto& p : properties) {
                // operator[] of an std::map<K,T> requires that T be default-constructible.
                // But reference_wrapper is not.
                // Thus, we must directly forward the reference.
                _properties.insert_or_assign(p.get().name(), std::ref(p.get()));
            }
        }
        
    public:
        /** Use this constructor if you just need to trigger a log event if ANY of the triggers are fired.
         *
         * @warning Do not pass references to members of your derived class,
         *          as this would pass a reference to an unitiliazed member,
         *          the base class constructor being always called first.
         *          Instead, use the empty `Logger()` constructor and then
         *          add the triggers manually in your own constructor.
         */
        Logger(std::vector<std::reference_wrapper<logger::Trigger >> triggers,
               std::vector<std::reference_wrapper<logger::Property>> properties)
        : _any(triggers)
        , _triggers(_any)
        , _problem(nullptr)
        {
            map_properties(properties);
            assert(consistent_properties());
        }

        /** Use this constructor if you want to combine triggers differently.
         *
         * You can for instance combine your triggers in a trigger::All,
         * so that the log event would be triggered only if ALL triggers
         * are fired at once.
         */
        Logger(trigger::Set& triggers,
               std::vector<std::reference_wrapper<logger::Property>> properties               )
        : _any()
        , _triggers(triggers)
        , _problem(nullptr)
        {
            map_properties(properties);
            assert(consistent_properties());
        }

        /** Use this constructor if you just need the interface without triggers or properties,
         * for instance if you define default triggers/properties in your constructor.
         * 
         * @note If you manage your own default triggers/properties,
         *       use `_properties.insert_or_assign(name, std::ref(tp));`
         *       instead of classical access operators.
         * 
         * Used by logger::Combine, for instance.
         */
        // _triggers needs to be a reference, because it's an interface.
        // thus we initialize it with an (empty) _any.
        Logger() : _any(), _triggers(_any), _problem(nullptr), _properties() {}

        /** Add the given trigger to the list. */
        void trigger(logger::Trigger when)
        {
            _triggers.push_back(when);
        }

        /** Check if the logger should be triggered and if so, call `call(log_info)`. */
        // This is virtual because logger::Combine needs to bypass the default behaviour.
        virtual void log(const logger::Info& log_info)
        {
            IOH_DBG(debug,"log " << log_info.raw_y_best << " => " << log_info.transformed_y << " / " << log_info.transformed_y_best);
            assert(_problem != nullptr); // For Debug builds.
            if(not _problem) { // For Release builds.
                throw std::runtime_error("Logger has not been attached to a problem.");
            }

            assert(_properties.size() > 0);
            assert(_triggers.size() > 0);
            if(_triggers(log_info, *_problem)) {
                IOH_DBG(debug,"logger triggered")
                call(log_info);
            }
        }

        /** Starts a new session for the given problem/instance/dimension/run.
         * 
         * This is called automatically by the Problem (or Suite) to which the Logger is attached,
         * each time a change occurs (be it a new run, or a new problem instance).
         * 
         * @warning If you override this function, do not forget to call the base class' one.
         */
        virtual void attach_problem(const problem::MetaData& problem)
        {
            IOH_DBG(xdebug,"attach problem " << problem.problem_id);
            _problem = &problem;
        }

        //! Starts a new session for the given Suite name.
        virtual void attach_suite(const std::string& suite_name) = 0;

        //! Main entry point, called everytime a Trigger is true.
        virtual void call(const logger::Info& log_info) = 0;

        /** Optional actions when the logger is detached from a suite/problem or the problem is reset.
         *
         * Useful if you want to flush data, for instance.
         * 
         * @warning If you override this function, do not forget to call the base class' one.
         */
        virtual void reset()
        {
            IOH_DBG(debug,"reset");
            _triggers.reset();
        }

        virtual ~Logger() = default;

        /** Access the attached problem's metadata. */
        problem::MetaData problem() const { return *_problem; }
    };

    /** Everything related to loggers. */
    namespace logger {

        /** Interface for a Logger to which the user can add properties to be watched.
         * 
         * @ingroup Logging
         */
        class Watcher: public Logger {
        public:
            /** Use this constructor if you just need to trigger a log event if ANY of the triggers are fired.
             *
             * @warning Do not pass references to members of your derived class,
             *          as this would pass a reference to an unitiliazed member,
             *          the base class constructor being always called first.
             *          Instead, use the empty `Watcher()` constructor and then
             *          add the properties manually in your own constructor.
             */
            Watcher(std::vector<std::reference_wrapper<logger::Trigger >> when,
                    std::vector<std::reference_wrapper<logger::Property>> what)
            : Logger(when,what)
            { }

            /** Use this constructor if you want to combine triggers differently.
             *
             * You can for instance combine your triggers in a trigger::All,
             * so that the log event would be triggered only if ALL triggers
             * are fired at once.
             */
            Watcher(trigger::Set& when,
                   std::vector<std::reference_wrapper<logger::Property>> what)
            : Logger(when,what)
            { }

            /** Use this constructor if you just need the interface without triggers or properties,
             * for instance if you define default triggers/properties in your own constructor.
             * 
             * @note If you manage your own default triggers/properties,
             *       use `_properties.insert_or_assign(name, std::ref(tp));`
             *       instead of classical access operators.
             */
             Watcher() : Logger() { }

            //! Adds a property to be logged.
            // This essentially just expose _properties.push_back with some checks.
            void watch(logger::Property& property)
            {
                IOH_DBG(debug,"watch property " << property.name());
                // Assert that the Property is not already tracked.
                assert(std::find_if(std::begin(_properties),std::end(_properties),
                                    [&property](const auto rwp){return property.name() == rwp.second.get().name();}
                                   ) == std::end(_properties));
                _properties.insert_or_assign(property.name(), property);
                assert(consistent_properties()); // Double check duplicates, in case the code above would be changed.
            }

            /* Interface from Logger to be implemented:
            virtual void track_problem(const problem::MetaData& problem) = 0;
            virtual void track_suite(const std::string& suite_name) = 0;
            virtual void log(const logger::Info& log_info) = 0;
            */
        };
        
    } // logger

}

#pragma once

#include <set>

// #include "api.hpp"
#include "../problem/utils.hpp"

namespace ioh {
    namespace logger {

        /** @defgroup Triggering Triggering
         * Everything related to the triggering API.
         *
         * @ingroup API
         */

        /** @defgroup Triggers Triggers
         * Events determining when to do a log event.
         * 
         * Examples:
         * @code
            // Fire log events only when the best transformed objective function value found so far has improved.
            ioh::logger::Store store_all_bests({ioh::trigger::on_improvement},{ioh::watch::transformed_y_best});

            // Do log regularly along the run, or if there is an improvement.
            ioh::logger::Store store_regularly({ // Several triggers defaults to trigger::any. Thus, fires if:
                    ioh::trigger::each(10),      // each 10 calls, OR...
                    ioh::trigger::on_improvement // on improvement.
                }, {ioh::watch::transformed_y_best});

            // Do log at the 10 first and 10 last calls of a 100-evaluations run.
            ioh::logger::Store store_some({ioh::trigger::during({{0,10},{90,100}})}, {ioh::watch::transformed_y_best});

            // Do log if there is improvement at the beginning or the end, but not in between.
            ioh::logger::Store complex_store({ioh::trigger::all({ // Fires only if all sub-triggers do fire, that is if:
                    ioh::trigger::on_improvement,                 // there is improvement, AND...
                    ioh::trigger::during({{0,10},{90,100}})       // at the beginning or end.
                }, {ioh::watch::transformed_y_best});
         * @endcode
         *
         * @warning Convenience instances are global variables provided for their ease of use.
         *          However, some of them manage a state (like trigger::on_improvement).
         *          Thus, think twice before using them in several loggers!
         *          In some rare cases, you may want to have two copies, to avoid side effects.
         *
         * Some triggers need to be instantiated with parameters before being passed to a logger,
         * you can use the related free functions (lower-case name) to do so on the heap.
         * 
         * @warning Those free functions do allocate on the heap, so you're responsible of freeing memory after them if necessary.
         *
         * For example:
             @code
                auto t& = trigger:at({1,10,100});
                // [Use t...]
                delete &t;
             @endcode
         * 
         * @ingroup Loggers
         */
        
        /** Interface for classes triggering a log event.
         *
         * @ingroup Triggering
         */
        struct Trigger {

            /** @returns true if a log event is to be triggered given the passed state. */
            virtual bool operator()(const logger::Info& log_info, const problem::MetaData& pb_info) = 0;

            /** Reset any internal state.
             * 
             * @note This is called when the logger is attached to a new problem/run/etc.
             * 
             * Useful if, for instance, the trigger maintain its own "best value so far" (@see logger::OnImprovement).
             */
            virtual void reset() { }
        };
        
    } // logger

    /** Everything related to triggering a logger event. */
    namespace trigger {

        /** Interface to combine several triggers in a single one.
         * 
         * By default, the Logger class manage combine several triggers with a logical "or" (see trigger::Any),
         * but you may want to do differently, in which case you can inherit
         * from this class to enjoy the triggers list management,
         * and just do what you want with `operator()`.
         * 
         * @ingroup Triggering
         */
        class Set : public logger::Trigger {
            protected:
                //! Managed triggers.
                std::vector<std::reference_wrapper<logger::Trigger>> _triggers;

            public:
                /** Empty constructor
                 * 
                 * @warning You should probably not use this one.
                 */
                // Empty constructor are needed (at least for the "_any" default member of Logger).
                Set(){}

                /** Constructor
                 * 
                 * @param triggers the managed triggers.
                 */
                Set( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
                : _triggers(triggers)
                { }

                // virtual bool operator()(const logger::Info& log_info, const problem::MetaData& pb_info) = 0;
            
                /** Propagate the reset event to all managed triggers. */
                virtual void reset() override
                {
                    IOH_DBG(debug,"reset triggers");
                    for(auto& trigger : _triggers) {
                        trigger.get().reset();
                    }
                }

                //! Add a trigger to the set.
                void push_back(logger::Trigger& trigger) { _triggers.push_back(std::ref(trigger)); }

                //! Add a trigger to the set.
                void insert(logger::Trigger& trigger) { _triggers.push_back(std::ref(trigger)); }

                //! Get the number of managed triggers.
                size_t size() { return _triggers.size(); }

        };

        /** Combine several triggers in a single one with a logical "or".
         * 
         * @note This is use as the default combination when you pass several triggers to a logger.
         * 
         * @ingroup Triggering
         */
        struct Any: public Set {

            /** Empty constructor
             * 
             * @warning You should probably not use this one.
             */
            Any():Set(){}

            /** Constructor
             * 
             * @param triggers the managed triggers.
             */
            Any( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
            : Set(triggers)
            { }

            /** Triggered if ANY the managed triggers are triggered. */
            virtual bool operator()(const logger::Info& log_info, const problem::MetaData& pb_info) override
            {
                assert(_triggers.size() > 0);
                for(auto& trigger : _triggers) {
                    if(trigger(log_info, pb_info)) {
                        IOH_DBG(debug,"any triggered");
                        return true;
                    }
                }
                IOH_DBG(xdebug,"not any triggered");
                return false;
            }
        };
        /** Do log if ANY of the given triggers is fired.
         * 
         * @ingroup Triggers
         */
        inline Any& any( std::vector<std::reference_wrapper<logger::Trigger>> triggers )
        {
            auto t = new Any(triggers);
            return *t;
        }
        
        /** Combine several triggers in a single one with a logical "and".
         * 
         * @ingroup Triggering
         */
        struct All: public Set {

            /** Empty constructor.
             * 
             * @warning You should probably not use this one directly.
             */
            All():Set(){}

            /** Constructor
             * 
             * @param triggers the set of Trigger to manage.
             */
            All( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
            : Set(triggers)
            { }

            /** Triggered if ALL the managed triggers are triggered. */
            virtual bool operator()(const logger::Info& log_info, const problem::MetaData& pb_info) override
            {
                assert(_triggers.size() > 0);
                for(auto& trigger : _triggers) {
                    if(not trigger(log_info, pb_info)) {
                        IOH_DBG(xdebug,"not all triggered");
                        return false;
                    }
                }
                IOH_DBG(debug,"all triggered");
                return true;
            }

        };
        /** Do log if ALL the given triggers are fired.
         *
         * @ingroup Triggers
         */
        inline All& all( std::vector<std::reference_wrapper<logger::Trigger>> triggers )
        {
            auto t = new All(triggers);
            return *t;
        }

        /** A trigger that always returns `true`.
         * 
         * @ingroup Triggering
         */
        struct Always : public logger::Trigger {
            bool operator()(const logger::Info&, const problem::MetaData&) override
            {
                IOH_DBG(debug,"always triggered");
                return true;
            }
        };
        /** Do log at every call of the objective function.
         *
         * @ingroup Triggers
         */
        // extern Always always; // Uncomment if one want a library.

        /** A trigger that react to a strict improvement of the best transformed value.
         * 
         * @ingroup Triggering
         */
        class OnImprovement : public logger::Trigger {
        protected:
            //! Local memory of the best value.
            double _best;
            //! Current problem type.
            common::OptimizationType _type;

            //! State management flag.
            // Avoids being dependent on a problem at construction, 
            // to the expense of a test at each call.
            bool _has_type;

        public:
            //! Constructor.
            OnImprovement()
            : _has_type(false)
            {
                reset();
            }

            //! Main call interface.
            bool operator()(const logger::Info& log_info, const problem::MetaData& pb_info) override
            {
                if(not _has_type) {
                    _type = pb_info.optimization_type;
                    if(_type == common::OptimizationType::Minimization) {
                        IOH_DBG(debug,"reconfigure problem type as minimization");
                        _best =  std::numeric_limits<double>::infinity();
                    } else {
                        IOH_DBG(debug,"reconfigure problem type as maximization");
                        _best = -std::numeric_limits<double>::infinity();
                    }
                    _has_type = true;
                }
                // We do not use logger::Info::transformed_y_best below,
                // because all fields of logger::Info are updated before the trigger see them.
                // That would force to test for equality to trigger on improvement,
                // and we only want to trigger on strict inequality.
                assert(_has_type);
                if(common::compare_objectives(log_info.transformed_y, _best, _type)) {
                    _best = log_info.transformed_y;
                    IOH_DBG(debug,"triggered on improvement by " << log_info.transformed_y << " / " << _best);
                    return true;
                }
                IOH_DBG(xdebug,"not triggered on improvement by " << log_info.transformed_y << " / " << _best);
                return false;
            }

            //! Forget previous state.
            void reset() override
            {
                _has_type = false;
            }
        };
        /** Do log only if the transformed best objective function value found so far has strictly improved.
         * 
         * @ingroup Triggers
         */
        // extern OnImprovement on_improvement; // Uncomment if one want a library.

        /** A trigger that fire at a regular interval.
         * 
         * @ingroup Triggering
         */
        class Each : public logger::Trigger {
        protected:
            //! Period of time between triggers.
            const size_t _interval;
            //! Minimum time at which to allow triggering.
            const size_t _starting_at;

        public:
            /** Constructor
             * 
             * @param interval number of evaluations between two triggering events.
             * @param starting_at minimum time at which to start triggering events.
             */
            Each(const size_t interval, const size_t starting_at = 0)
            : _interval(interval)
            , _starting_at(starting_at)
            { }

            //! Main call interface.
            bool operator()(const logger::Info& log_info, const problem::MetaData&) override
            {
                if((log_info.evaluations-_starting_at) % _interval == 0) {
                    IOH_DBG(debug,"each triggered " << log_info.evaluations);
                    return true;
                } else {
                    IOH_DBG(xdebug,"each not triggered " << log_info.evaluations);
                    return false;
                }
            }
        };
        /** Do log every given number of function evaluations.
         * 
         * Starts counting at zero by default.
         * 
         * @ingroup Triggers
         */
        inline Each& each(const size_t interval, const size_t starting_at = 0)
        {
           auto t = new Each(interval, starting_at);
           return *t;
        }

        /** A trigger that fire only at specific number of evaluations.
         *
         * @ingroup Triggering
         */
        class At : public logger::Trigger {
        protected:
            //! Set of times at which to trigger events.
            const std::set<size_t> _time_points;

            //! Check if time is in the managed set.
            bool matches(const size_t evals)
            {
                return _time_points.find(evals)
                       != std::end(_time_points);
            }

        public:
            /** Constructor.
             * 
             * @param time_points the set of evaluations for which to trigger an event.
             */
            At(const std::set<size_t> time_points)
            : _time_points(time_points)
            { }

            //! Main call interface.
            bool operator()(const logger::Info& log_info, const problem::MetaData&) override
            {
                if(matches(log_info.evaluations)) {
                    IOH_DBG(debug,"triggered at " << log_info.evaluations);
                    return true;
                } else {
                    IOH_DBG(xdebug,"not triggered at " << log_info.evaluations);
                    return false;
                }
            }
        };
        /** Do log at the given number of evaluations.
         * 
         * @ingroup Triggers
         */
        inline At& at(const std::set<size_t> time_points)
        {
            auto t = new At(time_points);
            return *t;
        }

        /** A trigger that fire only when the number of evaluations is within certain range(s).
         *
         * @note Ranges are closed intervals: with `[start,end]`, the trigger will fire for start and end.
         *       Ranges may overlap with each other.
         * 
         * @ingroup Triggering
         */
        class During : logger::Trigger {
        protected:
            //! Time ranges during which events are triggered.
            const std::set<std::pair<size_t,size_t>> _time_ranges;

            //! Check if a time is in one of the ranges.
            bool matches(const size_t evals)
            {
                // TODO Use binary search to speed-up ranges tests.
                for(const auto& r : _time_ranges) {
                    assert(r.first <= r.second);
                    if(r.first <= evals and evals <= r.second) {
                        return true;
                    }
                }
                return false;
            }

        public:
            /** Constructor.
             * 
             * @param time_ranges the set of [min_evals,max_evals] during which events will be triggered.
             */
            During( std::set<std::pair<size_t,size_t>> time_ranges )
            : _time_ranges(time_ranges)
            {
#ifndef NDEBUG
                assert(not _time_ranges.empty());
                for(const auto& r : _time_ranges) {
                    assert(r.first <= r.second); // less or equal, because it can be a single time point.
                }
#endif
            }

            //! Main call interface.
            bool operator()(const logger::Info& log_info, const problem::MetaData&) override
            {
               if(matches(log_info.evaluations)) {
                   IOH_DBG(debug,"triggered during " << log_info.evaluations);
                   return true;
               } else {
                   IOH_DBG(xdebug,"not triggered during " << log_info.evaluations);
                   return false;
               }
            }

        };
        /** Do log if the number of evaluations falls in at least one of the given ranges.
         * 
         * @note Ranges are closed intervals `[start,end]`, the trigger will fire for start and end.
         *       Ranges may overlap with each other.
         * 
         * @ingroup Triggers
         */
        inline During& during(const std::set<std::pair<size_t,size_t>> time_ranges)
        {
            auto t = new During(time_ranges);
            return *t;
        }

    } // trigger
} // ioh

#pragma once

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
            ioh::logger::Store store_all_bests({ioh::trigger::always},{ioh::watch::transformed_y_best});
         * @endcode
         * 
         * @ingroup Loggers
         */
        
        /** Interface for classes triggering a log event.
         *
         * @ingroup Triggering
         */
        struct Trigger {

            /** @returns true if a log event is to be triggered given the passed state. */
            virtual bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) = 0;

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
                std::vector<std::reference_wrapper<logger::Trigger>> _triggers;

            public:
                // Empty constructor are needed (at least for the "_any" default member of Logger).
                Set(){}

                Set( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
                : _triggers(triggers)
                { }

                // virtual bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) = 0;
            
                /** Propagate the reset event to all managed triggers. */
                virtual void reset() override
                {
                    for(auto& trigger : _triggers) {
                        trigger.get().reset();
                    }
                }

                void push_back(logger::Trigger& trigger) { _triggers.push_back(trigger); }

                size_t size() { return _triggers.size(); }

        };

        /** Combine several triggers in a single one with a logical "or".
         * 
         * @note This is use as the default combination when you pass several triggers to a logger.
         * 
         * @ingroup Triggering
         */
        struct Any: public Set {

            Any():Set(){}

            Any( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
            : Set(triggers)
            { }

            /** Triggered if ANY the managed triggers are triggered. */
            virtual bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) override
            {
                for(auto& trigger : _triggers) {
                    if(trigger(log_info, pb_info)) {
                        return true;
                    }
                }
                return false;
            }
        };
        /** Do log if ANY of the given triggers is fired.
         * 
         * @ingroup Triggers
         */
        Any any( std::vector<std::reference_wrapper<logger::Trigger>> triggers )
        {
            return Any(triggers);
        }
        
        /** Combine several triggers in a single one with a logical "and".
         * 
         * @ingroup Triggering
         */
        struct All: public Set {

            All():Set(){}

            All( std::vector<std::reference_wrapper<logger::Trigger>> triggers)
            : Set(triggers)
            { }

            /** Triggered if ALL the managed triggers are triggered. */
            virtual bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) override
            {
                for(auto& trigger : _triggers) {
                    if(not trigger(log_info, pb_info)) {
                        return false;
                    }
                }
                return true;
            }

        };
        /** Do log if ALL the given triggers are fired.
         *
         * @ingroup Triggers
         */
        All all( std::vector<std::reference_wrapper<logger::Trigger>> triggers )
        {
            return Any(triggers);
        }

        /** A trigger that always returns `true`.
         * 
         * @ingroup Triggering
         */
        struct Always : public logger::Trigger {
            bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) override
            {
                return true;
            }
        };
        /** Log at every call of the objective function.
         *
         * @ingroup Triggers
         */
        Always always;

        /** A trigger that react to a strict improvement of the best transformed value.
         * 
         * @ingroup Triggering
         */
        class OnImprovement : public logger::Trigger {
        protected:
            double _best;
            const common::OptimizationType _type;
        public:
            OnImprovement(const problem::MetaData& pb_info) // FIXME there should be a way to get rid of the instantiation dependency.
            : _type(pb_info.optimization_type)
            {
                reset();
            }
            
            bool operator()(const log::Info& log_info, const problem::MetaData& pb_info) override
            {
                // We do not use log::Info::transformed_y_best below,
                // because all fields of log::Info are updated before the trigger see them.
                // That would force to test for equality to trigger on improvement,
                // and we only want to trigger on strict inequality.
                if(common::compare_objectives(log_info.transformed_y, _best, _type)) {
                    _best = log_info.transformed_y;
                    return true;
                }
                return false;
            }

            void reset() override
            {
                if(_type == common::OptimizationType::Minimization) {
                    _best =  std::numeric_limits<double>::infinity();
                } else {
                    _best = -std::numeric_limits<double>::infinity();
                }
            }
        };


        // TODO HERE: AtInterval AtTimePoints PerTimeRange
    } // trigger
} // ioh
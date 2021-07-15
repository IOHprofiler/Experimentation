#pragma once

#include <optional>

#include "loginfo.hpp"

namespace ioh {
    namespace logger {

        /** @defgroup Properties Properties
         * Accessors to values that are to be logged.
         * 
         * You can directly pass the "variables" to a logger to make it track their values.
         * 
         * @note Only some loggers are able to track user-defined properties
         *       (those inheriting from the Watcher interface).
         * 
         * @note The loggers only watch variables that may be casted to `double`.
         *       The functions are templated so that you don't need to do the casting by yourself,
         *       but it is nonetheless mandatory.
         * 
         * For example:
         * @code
                double my_algo_parameter = 42;

                // Track the number of evaluations and the best value of the objective function
                // (at each calls with the Store logger):
                ioh::logger::Store my_store( // Log everything in-memory.
                    {ioh::trigger::always},                                     // Trigger the logger at each call.
                    {                                                           // Do log:
                        ioh::watch::evaluations,                                // - evaluations,
                        ioh::watch::transformed_y_best,                         // - best values,
                        ioh::watch::reference("my_parameter",my_algo_parameter) // - any change to my extern variable.
                    }
                );
         * @endcode
         * 
         * Some properties need to be instantiated with parameters before being passed to a logger,
         * you can use the related free functions (lower-case names) to do so on the heap.
         * 
         * @warning Those free functions do allocates on the heap, so you're responsible of freeing memory after them if necessary.
         *
         * For example:
         * @code
                auto& p = watch::reference("my", some_variable);
                // [Use p...]
                delete &p;
         * @endcode
         * 
         * @ingroup Loggers
         */

        /** Interface for callbacks toward variable to be logged.
         * 
         * A class that inherit this interface
         * should be able to indicate if the managed variable
         * is not available at the time of the current logger's call.
         * 
         * @ingroup Logging
         */
        class Property{
        protected:
            //! Unique name for the logged property.
            const std::string _name;
        public:
            //! Constructor.
            Property(const std::string name)
            : _name(name)
            {
                assert(_name.size() > 0);
            }

            /** Returns the current value of the managed variable
             * 
             * The call interface should return an optional,
             * which should be set to `std::nullopt`
             * if the managed variable cannot be accessed
             * (for instance if it does not exists during the call scope,
             *  for example if it's a dynamic algorithm parameter that
             *  is not currently configured).
             * 
             * @param log_info The current problem state data.
             * @returns An optional that holds a `double` if the variable is available, `std::nullopt` else.
             */
            virtual std::optional<double> operator()(const logger::Info& log_info) const = 0;

            //! Configured name accessor.
            std::string name() const { return _name; }
        };

    } // logger

    /** Properties that can be watched by loggers. */
    namespace watch {

        /** A property that access the number of evaluations so far.
         * 
         * @ingroup Logging
         */
        struct Evaluations : public logger::Property {
            //! Constructor.
            Evaluations(const std::string name = "evaluations") : logger::Property(name) {}
            //! Main call interface.
            std::optional<double> operator()(const logger::Info& log_info) const
            {
                // This should always be accessible, so the optional will always contain the variable.
                return std::make_optional(static_cast<double>(log_info.evaluations));
            }
        };
        /** Number of evaluations of the objective function called by the solver.
         * 
         * @ingroup Properties
         */
        // extern Evaluations evaluations; // Uncomment if one want a library.
        
        /** A property that access the best value so far, without transformation.
         *
         * @ingroup Logging
         */
        struct RawYBest: public logger::Property {
            //! Constructor.
            RawYBest(const std::string name = "raw_y_best") : logger::Property(name) {}
            //! Main call interface.
            std::optional<double> operator()(const logger::Info& log_info) const
            {
                return std::make_optional(log_info.raw_y_best);
            }
        };
        /** Best objective function value found so far, without transformation.
         * 
         * @ingroup Properties
         */
         // extern RawYBest raw_y_best; // Uncomment if one want a library.

        /** A property that access the current value so far, with transformation.
         *
         * @ingroup Logging
         */
        struct TransformedY: public logger::Property {
            //! Constructor.
            //! Main call interface.
            TransformedY(const std::string name = "transformed_y") : logger::Property(name) {}
            std::optional<double> operator()(const logger::Info& log_info) const
            {
                return std::make_optional(log_info.transformed_y);
            }
        };
        /** Objective function value for this call, with transformation.
         * 
         * @ingroup Properties
         */
        // extern TransformedY transformed_y; // Uncomment if one want a library.

        /** A property that access the best value found so far, with transformation.
         *
         * @ingroup Logging
         */
        struct TransformedYBest: public logger::Property {
            //! Constructor.
            TransformedYBest(const std::string name = "transformed_y_best") : logger::Property(name) {}
            //! Main call interface.
            std::optional<double> operator()(const logger::Info& log_info) const
            {
                return std::make_optional(log_info.transformed_y_best);
            }
        };
        /** Best objective function value found so far, with transformation. 
         * 
         * @ingroup Properties
         */
        // extern TransformedYBest transformed_y_best; // Uncomment if one want a library.

        /** A property that access a referenced variable.
         *
         * @note The variable must be castable to a double.
         * 
         * @ingroup Logging
         */
        template<class T>
        class Reference: public logger::Property {
            protected:
                //! The managed reference.
                const T& _variable;
                
            public:
                /** Constructor.
                 *
                 * @param name the name of the property.
                 * @param variable a reference to the logged variable.
                 */
                Reference(const std::string name, const T& variable)
                : logger::Property(name),
                _variable(variable)
                { }

                //! Main call interface.
                std::optional<double> operator()(const logger::Info&) const
                {
                    return std::make_optional(static_cast<double>(_variable));
                }
        };
        /** The value of an extern variable (captured by reference).
         *
         * @param name the name of the property.
         * @param variable a reference to the logged variable.
         *
         * @ingroup Properties
         */
         template<class T>
         Reference<T>& reference(const std::string name, const T& variable)
         {
            auto p = new Reference<T>(name,variable);
            return *p;
         }

        /** A property that access a variable through a pointer.
         *
         * @note The pointed variable must be castable to a double.
         *
         * @ingroup Logging
         */
        template<class T>
        class Pointer: public logger::Property {
            protected:
                //! The managed variable.
                const T* const _variable;
                
            public:
                /** Constructor.
                 * 
                 * @param name the name of the property.
                 * @param variable a pointer to the logged variable.
                 */
                Pointer(const std::string name, const T* const variable)
                : logger::Property(name),
                _variable(variable)
                {
                    assert(variable != nullptr);
                }
                
                //! Main call interface.
                std::optional<double> operator()(const logger::Info&) const
                {
                    return std::make_optional(static_cast<double>(*_variable));
                }
        };
        /** The value of an extern variable (captured by address).
         *
         * @param name the name of the property.
         * @param variable a pointer to the logged variable.
         * 
         * @ingroup Properties
         */
         template<class T>
         Pointer<T>& address(const std::string name, const T* const variable)
         {
            auto p = new Pointer<T>(name,variable);
            return *p;
         }

        /** A property that access the variable of a variable through a referente to a pointer.
         *
         * Use this if the variable does not always exists in the logged scope.
         * 
         * @warning It is your responsability to ensure that the referenced pointer is a nullptr if the variable is out of the logged scope.
         * 
         * @note The pointed variable must be castable to a double.
         *
         * @ingroup Logging
         */
        template<class T>
        class PointerReference: public logger::Property {
            protected:
                //! The managed reference to a pointer.
                const T* const & _variable;
                
            public:

                /** Constructor.
                 * 
                 * @param name the name of the property.
                 * @param variable a reference to a pointer to the logged variable.
                 */
                PointerReference(const std::string name, const T* const & variable)
                : logger::Property(name),
                _variable(variable)
                { }
                
                //! Main call interface.
                std::optional<double> operator()(const logger::Info&) const
                {
                    if(_variable != nullptr) {
                        return std::make_optional(static_cast<double>(*_variable));
                    } else {
                        return std::nullopt;
                    }
                }
        };
        /** The value of an extern variable, which may not exists.
         * 
         * Useful for variables that doesn't exists at some point during the solver run.
         * In that case, if the referenced pointer is a `std::nullptr`,
         * the value will be indicated as invalid in the logs.
         *
         * To do so, this captures a reference to a pointer toward your variable.
         * If you update the value of the pointed variables, it will change in the logs.
         * If you need to invalidate the variable, you can set the referenced pointer itself to `nullptr`.
         * 
         * @warning It is your responsability to ensure that the referenced pointer is a nullptr if the variable is out of the logged scope.
         * 
         * @param name the name of the property.
         * @param variable a reference to a pointer to the logged variable.
         * 
         * @ingroup Properties
         */
         template<class T>
         PointerReference<T>& pointer(const std::string name, const T* const & variable)
         {
            auto p = new PointerReference<T>(name,variable);
            return *p;
         }

    } // watch
} // ioh

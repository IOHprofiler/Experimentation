#pragma once

#include <stdexcept>

#include "../problem/utils.hpp"

namespace ioh {

    /** Shared structure related to loggers. */
    namespace logger {

        /** Information about the current log.
         * 
         * @note The properties for bests values holds a state since the first start or the last call to reset.
         * 
         * @note "transformed" indicates that a monotonic transformation is applied,
         *       which is specific to the currently configured problem _instance_.
         * 
         * @note If there is an improvement, then fields holding "best" values will have the same value than the others.
         * 
         * @ingroup Logging
        */
        struct Info
        {
            //! Number of evaluations of the objective function so far.
            size_t evaluations;

            //! The current best internal objective function value (since the last reset).
            double raw_y_best; // was y_best
            
            //! The current transformed objective function value.
            double transformed_y;
            
            //! The current best transformed objective function value (since the last reset).
            double transformed_y_best;
            
            //! Currently considered solution with the corresponding transformed objective function value.
            problem::Solution<double> current;
            
            //! Optimum to the current problem instance, with the corresponding transformed objective function value.
            problem::Solution<double> optimum; // was objective
        };

    } // log
} // ioh


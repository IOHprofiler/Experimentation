#pragma once

#include "logger/loginfo.hpp"
#include "logger/properties.hpp"
#include "logger/triggers.hpp"
#include "logger/loggers.hpp"
#include "logger/store.hpp"
#include "logger/flatfile.hpp"
#include "logger/combine.hpp"
#include "logger/eah.hpp"
#include "logger/eaf.hpp"

/** @defgroup Loggers Loggers
 * Objects that track the calls to the objective function.
 * 
 * @ingroup Features
 * 
 * The loggers are attached to a problem and each time a solver calls its objective function,
 * a logging event may be triggered.
 * 
 */

/** @defgroup Logging Logging
 * Everything related to the logging API.
 * 
 * @ingroup API
 */


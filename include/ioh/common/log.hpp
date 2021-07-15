#pragma once

#include <ostream>
#include <iostream>
#include <string>

#include <clutchlog/clutchlog.h>

#define IOH_DBG CLUTCHLOG
#define IOH_DBG_DUMP CLUTCHDUMP

// namespace ioh::common::log {
//     /**
//      * \brief Enum containing log levels; info = 0, warning = 1, error = 2
//      */
//     enum class Level {
//         Info,
//         Warning,
//         Error
//     };

//     /**
//      * \brief Setting for the log level
//      */
//     inline Level log_level = Level::Info;

//     /**
//      * \brief Function for logging errors, causes a system exit
//      * \param error_msg The error message
//      * \param os The stream to log the messages to
//      */
//     inline void error(const std::string &error_msg, std::ostream &os = std::cerr)
//     {
//         // TODO: Raise Exception
//         if (log_level <= Level::Error) {
//             os << "IOH_ERROR_INFO : " << error_msg << "\n";
//             std::flush(os);
//             std::flush(std::cerr);
//             std::flush(std::cout);
//             exit(1);
//         }
//     }

//     /**
//      * \brief Function for logging warnings
//      * \param warning_msg The error message
//      * \param os The stream to log the messages to
//      */
//     inline void warning(const std::string &warning_msg, std::ostream &os = std::cout)
//     {
//         if (log_level <= Level::Warning)
//             os << "IOH_WARNING_INFO : " << warning_msg << "\n";
//     }

//     /**
//      * \brief Function for logging info messages to a stream
//      * \param log_msg The info message
//      * \param os The stream to log the messages to
//      */
//     inline void info(const std::string &log_msg, std::ostream &os = std::cout) {
//         if (log_level <= Level::Info)
//             os << "IOH_LOG_INFO : " << log_msg << "\n";
//     }
// }

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "ioh.hpp"

namespace py = pybind11;
//
// struct LogInfo
// {
//     size_t evaluations;
//     double y_best;
//     double transformed_y;
//     double transformed_y_best;
//     problem::Solution<double> current;
//     problem::Solution<double> objective;
// };

// struct Base {
//     virtual void track_problem(const problem::MetaData& problem) = 0;
//
//     virtual void track_suite(const std::string& suite_name) = 0;
//
//     virtual void log(const LogInfo& log_info) = 0;
//
//     virtual void flush() = 0;
// };


void init_logger(py::module&)
{
	

};
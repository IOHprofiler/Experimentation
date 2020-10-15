/// \file csv_logger.h
/// \brief Header file for class csv_logger.
///
/// \author Furong Ye
#pragma once

#include "observer.hpp"
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

namespace ioh
{
	namespace logger
	{
		/// \brief A class of logging csv files.
		///
		/// To activate logger functions as evaluating problems,  a 'logger' must be added to
		/// problem by the statement 'problem.add_logger(logger)'.
		template <class T>
		class csv : public observer<T>
		{
			// The information for directory.
			std::string folder_name;
			std::string output_directory;
			fs::path output_path;

			// The information for logging.
			std::string algorithm_name;
			std::string algorithm_info;
			common::optimization_type maximization_minimization_flag = common::optimization_type::maximization;
			std::map<std::string, std::string> attr_per_exp_name_value;
			std::map<std::string, std::shared_ptr<double>> attr_per_run_name_value;

			std::string suite_name = "No suite";

			int dimension;
			int problem_id;
			int instance;
			std::string problem_name;

			std::vector<double> best_y;
			std::vector<double> best_transformed_y;
			size_t optimal_evaluations;
			std::vector<double> last_y;
			std::vector<double> last_transformed_y;
			size_t last_evaluations;

			std::map<std::string, std::shared_ptr<double>> logging_parameters;

			// fstream
			std::fstream cdat;
			std::fstream idat;
			std::fstream dat;
			std::fstream tdat;
			std::fstream info_file;

			std::string cdat_buffer;
			std::string idat_buffer;
			std::string dat_buffer;
			std::string tdat_buffer;
			std::string info_buffer;


			int last_dimension = 0;
			int last_problem_id = -1;

			bool header_flag; /// < parameters to track if the header line is logged.


			/// \fn std::string experiment_folder_name()
			/// \brief return an available name of folder to be created.
			///
			/// To create a name of the folder logging files.
			/// If there exists a files or a folder with the same name, the expected 
			/// directory will be renamed by adding a suffix.
			/// For example,
			///     If a folder or file 'test' has already been in correct path, the 
			///     expected directory will be renamed as 'test-1', 'test-2', ... 
			///     until there is no such a folder or file. 
			fs::path experiment_folder_name()
			{
				auto renamed_directory = output_path / folder_name;
				auto temp_folder_name = folder_name;
				auto index = 0;
				while (exists(renamed_directory))
				{
					++index;
					temp_folder_name = folder_name + '-' + common::to_string(index);
					renamed_directory = output_path / temp_folder_name;
				}
				folder_name = temp_folder_name;
				return renamed_directory;
			}

			bool create_folder(const fs::path path) const
			{
				try
				{
					return create_directories(path);
				}
				catch (const std::exception& e)
				{
					common::log::error(std::string("Error on creating directory:") + e.what());
					return false;
				}
			}

			std::string dat_header() const
			{
				std::string dat_header =
					"\"function evaluation\" \"current f(x)\" \"best-so-far f(x)\" \"current af(x)+b\"  \"best af(x)+b\"";
				if (logging_parameters.size() != 0)
					for (auto const& e : logging_parameters)
						dat_header += " \"" + e.first + "\"";
				return dat_header;
			}

			fs::path sub_directory() const
			{
				return output_path / folder_name /
					("data_f" + common::to_string(problem_id) + "_" + problem_name);
			}

			void recreate_handle(const bool execute, const std::string ext, std::fstream& handle, std::string& buffer)
			{
				if (execute)
				{
					fs::path file_path = sub_directory() /
						("IOHprofiler_f" + common::to_string(problem_id) + "_DIM" + common::to_string(dimension) + ext);

					if (handle.is_open())
						handle.close();
					handle.open(file_path.c_str(), std::ofstream::out | std::ofstream::app);
					buffer = "";
					write_in_buffer(dat_header() + "\n", buffer, handle);
				}
			}

			void write_handle(std::fstream& handle, std::string& buffer)
			{
				if (handle.is_open())
				{
					write_stream(buffer, handle);
					handle.flush();
					buffer.clear();
				}
			}

			void write_handle(std::fstream& handle, std::string& buffer, std::string& line)
			{
				if (handle.is_open())
				{
					write_in_buffer(line, buffer, handle);
					write_handle(handle, buffer);
				}
			}

			void write_header()
			{
				if (!exists(sub_directory()))
					create_folder(sub_directory());

				recreate_handle(this->complete_status(), ".cdat", cdat, cdat_buffer);
				recreate_handle(this->interval_status(), ".idat", idat, idat_buffer);
				recreate_handle(this->update_status(), ".dat", dat, dat_buffer);
				recreate_handle(this->time_points_status(), ".tdat", dat, dat_buffer);
			}

			void write_stream(std::string buffer_string, std::fstream& dat_stream)
			{
				dat_stream.write(buffer_string.c_str(), sizeof(char) * buffer_string.size());
			}

			void write_in_buffer(std::string add_string, std::string& buffer_string, std::fstream& dat_stream)
			{
				if (!dat_stream.is_open())
				{
					common::log::error("file is not open");
					if (buffer_string.size() + add_string.size() < MAX_BUFFER_SIZE)
					{
						buffer_string = buffer_string + add_string;
					}
					else
					{
						write_stream(buffer_string, dat_stream);
						buffer_string.clear();
						buffer_string = add_string;
					}
				}
			}

			void conditional_write_in_buffer(const bool write, std::string add_string, std::string& buffer_string,
			                                 std::fstream& dat_stream)
			{
				if (write)
					write_in_buffer(add_string, buffer_string, dat_stream);
			}

			/// \fn openIndex()
			/// \brief to create the folder of logging files.
			bool open_index()
			{
				return create_folder(experiment_folder_name());
			}

		public:

			csv() :
				folder_name{"IOHprofiler_test"},
				output_directory{"./"},
				output_path{"./"},
				algorithm_name{"algorithm"},
				algorithm_info{"algorithm_info"}
			{
			}

			csv(std::string directory, std::string folder_name,
			    std::string alg_name, std::string alg_info) :
				folder_name{folder_name},
				output_directory{directory},
				output_path{directory},
				algorithm_name{alg_name},
				algorithm_info{alg_info}
			{
			}

			~csv()
			{
			}

			void clear_logger()
			{
				if (info_file.is_open())
				{
					write_info(this->instance, this->best_y[0], this->best_transformed_y[0], this->optimal_evaluations,
					           this->last_y[0], this->last_transformed_y[0], this->last_evaluations);
					info_file.close();
				}

				/// Close .dat files after .info file to record evaluations of the last iteration.
				if (cdat.is_open())
					cdat.close();
				if (idat.is_open())
					idat.close();
				if (dat.is_open())
					dat.close();
				if (tdat.is_open())
					tdat.close();
			}

			template <typename V>
			void add_attribute(const std::string name, const V value)
			{
				this->attr_per_exp_name_value[name] = common::to_string(value);
			}

			void delete_attribute(const std::string name)
			{
				this->attr_per_exp_name_value.erase(name);
			}

			/// \fn track_problem(int problem_id, int dimension, int get)
			///
			/// This function is to be invoked by problem class.
			/// To update info of current working problem, and to write headline in corresponding files.

			void track_problem(int problem_id, int dimension, int instance,
			                   std::string problem_name, common::optimization_type maximization_minimization_flag)
			{
				/// Handle info of the previous problem.
				if (info_file.is_open())
					write_info(instance, best_y[0], best_transformed_y[0], optimal_evaluations,
					           last_y[0], last_transformed_y[0], last_evaluations);

				optimal_evaluations = 0;
				last_evaluations = 0;

				/// TO DO: Update the method of initializing this value.
				best_y.clear();
				best_transformed_y.clear();
				last_y.clear();
				last_transformed_y.clear();
				if (maximization_minimization_flag == common::optimization_type::maximization)
				{
					best_y.push_back(-std::numeric_limits<double>::max());
					best_transformed_y.push_back(-std::numeric_limits<double>::max());
					last_y.push_back(-std::numeric_limits<double>::max());
					last_transformed_y.push_back(-std::numeric_limits<double>::max());
				}
				else
				{
					best_y.push_back(std::numeric_limits<double>::max());
					best_transformed_y.push_back(std::numeric_limits<double>::max());
					last_y.push_back(std::numeric_limits<double>::max());
					last_transformed_y.push_back(std::numeric_limits<double>::max());
				}

				this->reset_observer(maximization_minimization_flag);
				this->problem_id = problem_id;
				this->dimension = dimension;
				this->instance = instance;
				this->problem_name = problem_name;
				this->maximization_minimization_flag = maximization_minimization_flag;

				open_info(problem_id, dimension, problem_name);
				header_flag = false;
			}

			void track_problem(const T& problem) override
			{
				track_problem(
					problem.get_problem_id(),
					problem.get_number_of_variables(),
					problem.get_instance_id(),
					problem.get_problem_name(),
					problem.get_optimization_type()
				);
			}

			void track_suite(std::string suite_name)
			{
				this->suite_name = suite_name;
			}

			void track_suite(const suite::base<T>& suite) override
			{
				track_suite(suite.get_suite_name());
			}

			void open_info(int problem_id, int dimension, std::string problem_name)
			{
				this->info_buffer = "";
				std::string titleflag = "";
				std::string optimization_type = maximization_minimization_flag ==
				                                common::optimization_type::maximization
					                                ? "T"
					                                : "F";

				if (problem_id != this->last_problem_id)
				{
					this->info_file.close();
					fs::path infofile_path = output_path / folder_name /
						("IOHprofiler_f" + common::to_string(problem_id) + "_" + problem_name + ".info");


					if (exists(infofile_path))
						titleflag = "\n";

					this->info_file.open(infofile_path.c_str(), std::ofstream::out | std::ofstream::app);
					this->info_buffer += titleflag;

					this->info_buffer += ("suite = \"" + this->suite_name + "\", funcId = " +
						common::to_string(problem_id) + ", funcName = \"" + problem_name + "\", DIM = " +
						common::to_string(dimension) + ", maximization = \"" + optimization_type + "\", algId = \"" +
						this->algorithm_name + "\", algInfo = \"" + this->algorithm_info + "\"");


					if (this->attr_per_exp_name_value.size() != 0)
					{
						for (const auto& e : attr_per_exp_name_value)
							this->info_buffer += (", " + e.first + " = \"" + e.second + "\"");
					}

					if (this->attr_per_run_name_value.size() != 0)
					{
						this->info_buffer += ", dynamicAttribute = \"";
						for (std::map<std::string, std::shared_ptr<double>>::iterator iter = this->
							     attr_per_run_name_value.begin(); iter != this->attr_per_run_name_value.end();)
						{
							this->info_buffer += (iter->first);
							if (++iter != this->attr_per_run_name_value.end())
							{
								this->info_buffer += ("|");
							}
						}
						this->info_buffer += ("\"");
					}
					this->info_buffer += ("\n%\n");
					this->info_buffer += ("data_f" + common::to_string(problem_id) + "_" + problem_name +
						"/IOHprofiler_f" + common::to_string(problem_id) + "_DIM" + common::to_string(dimension) +
						".dat");
					this->write_stream(this->info_buffer, this->info_file);
					this->info_buffer.clear();

					this->last_problem_id = problem_id;
					this->last_dimension = dimension;
				}
				else if (dimension != this->last_dimension)
				{
					this->info_buffer += ("\nsuite = \"" + this->suite_name + "\", funcId = " +
						common::to_string(problem_id) + ", funcName = \"" + problem_name + "\", DIM = " +
						common::to_string(dimension) + ", maximization = \"" + optimization_type + "\", algId = \"" +
						this->algorithm_name + "\", algInfo = \"" + this->algorithm_info + "\"");
					if (this->attr_per_exp_name_value.size() != 0)
					{
						for (const auto& [fst, snd] : attr_per_exp_name_value)
							this->info_buffer += (", " + fst + " = \"" + snd + "\"");
					}

					if (this->attr_per_run_name_value.size() != 0)
					{
						this->info_buffer += (", dynamicAttribute = \"");
						for (std::map<std::string, std::shared_ptr<double>>::iterator iter = this->
							     attr_per_run_name_value.begin(); iter != this->attr_per_run_name_value.end();)
						{
							this->info_buffer += (iter->first);
							if (++iter != this->attr_per_run_name_value.end())
							{
								this->info_buffer += ("|");
							}
						}
						this->info_buffer += "\"";
					}
					this->info_buffer += "\n%\n";
					this->info_buffer += ("data_f" + common::to_string(problem_id) + "_" + problem_name +
						"/IOHprofiler_f" + common::to_string(problem_id) + "_DIM" + common::to_string(dimension) +
						".dat");
					this->write_stream(this->info_buffer, this->info_file);
					this->info_buffer.clear();
					this->last_problem_id = problem_id;
					this->last_dimension = dimension;
				}
			}

			void write_info(const int instance, const double best_y, const double best_transformed_y,
			                const int evaluations,
			                const double last_y, const double last_transformed_y, const int last_evaluations)
			{
				if (!info_file.is_open())
					common::log::error("write_info(): writing info into unopened info_file");

				this->info_buffer = "";
				this->info_buffer += (", " + common::to_string(instance) + ":" + common::to_string(evaluations) + "|" +
					common::to_string(best_y));
				if (this->attr_per_run_name_value.size() != 0)
				{
					this->info_buffer += (";");
					for (std::map<std::string, std::shared_ptr<double>>::iterator iter = this->attr_per_run_name_value.
						     begin(); iter != this->attr_per_run_name_value.end();)
					{
						this->info_buffer += (common::to_string(*(iter->second)));
						if (++iter != this->attr_per_run_name_value.end())
						{
							this->info_buffer += ("|");
						}
					}
				}
				this->write_stream(this->info_buffer, this->info_file);
				this->info_buffer.clear();


				if (evaluations != last_evaluations) // need write
				{
					auto written_line = common::to_string(last_evaluations) + " " + common::to_string(last_y) +
						" "
						+ common::to_string(best_y) + " " + common::to_string(last_transformed_y) + " "
						+ common::to_string(best_transformed_y);

					if (this->logging_parameters.size() != 0)
					{
						for (std::map<std::string, std::shared_ptr<double>>::iterator iter = this->logging_parameters.
							     begin(); iter != this->logging_parameters.end(); ++iter)
						{
							written_line += " ";
							written_line += common::to_string(*(iter->second));
						}
					}

					written_line += '\n';
					this->write_handle(cdat, cdat_buffer, written_line);
					this->write_handle(idat, idat_buffer, written_line);
					this->write_handle(dat, dat_buffer, written_line);
					this->write_handle(tdat, tdat_buffer, written_line);
				}

				this->write_handle(cdat, cdat_buffer);
				this->write_handle(idat, idat_buffer);
				this->write_handle(dat, dat_buffer);
				this->write_handle(tdat, tdat_buffer);
			}


			void write_line(size_t evaluations, double y, double best_so_far_y,
			                double transformed_y, double best_so_far_transformed_y)
			{
				if (header_flag == false)
				{
					this->write_header();
					header_flag = true;
				}

				this->last_evaluations = evaluations;
				this->last_y[0] = y;
				this->last_transformed_y[0] = transformed_y;

				bool cdat_flag = this->complete_trigger();
				bool idat_flag = this->interval_trigger(evaluations);
				bool dat_flag = this->update_trigger(transformed_y, maximization_minimization_flag);
				bool tdat_flag = this->time_points_trigger(evaluations);

				bool need_write = cdat_flag || idat_flag || dat_flag || tdat_flag;

				if (need_write)
				{
					std::string written_line = common::to_string(evaluations) + " " + common::to_string(y) + " "
						+ common::to_string(best_so_far_y) + " " + common::to_string(transformed_y) + " "
						+ common::to_string(best_so_far_transformed_y);

					if (this->logging_parameters.size() != 0)
					{
						for (auto const& e : logging_parameters)
							written_line += (" " + common::to_string(*(e.second)));
					}

					written_line += '\n';
					conditional_write_in_buffer(cdat_flag, written_line, cdat_buffer, cdat);
					conditional_write_in_buffer(idat_flag, written_line, idat_buffer, idat);
					conditional_write_in_buffer(dat_flag, written_line, dat_buffer, dat);
					conditional_write_in_buffer(tdat_flag, written_line, tdat_buffer, tdat);
				}

				if (common::compare_objectives(transformed_y, this->best_transformed_y[0],
				                               this->maximization_minimization_flag))
					this->update_logger_info(evaluations, y, transformed_y);
			}

			void do_log(const std::vector<double>& log_info) override
			{
				this->write_line(static_cast<size_t>(log_info[0]), log_info[1], log_info[2], log_info[3], log_info[4]);
			}

			void write_line(const std::vector<double>& log_info)
			{
				this->write_line(static_cast<size_t>(log_info[0]), log_info[1], log_info[2], log_info[3], log_info[4]);
			}

			void update_logger_info(size_t optimal_evaluations, double y, double transformed_y)
			{
				this->optimal_evaluations = optimal_evaluations;
				this->best_y[0] = y;
				this->best_transformed_y[0] = transformed_y;
			}

			void set_parameters(const std::vector<std::shared_ptr<double>>& parameters)
			{
				if (this->logging_parameters.size() != 0)
					this->logging_parameters.clear();

				for (size_t i = 0; i != parameters.size(); i++)
					this->logging_parameters["parameter" + common::to_string(i + 1)] = parameters[i];
			}

			void set_parameters(const std::vector<std::shared_ptr<double>>& parameters,
			                    const std::vector<std::string>& parameters_name)
			{
				if (parameters_name.size() != parameters.size())
					common::log::error("Parameters and their names are given with different size.");

				if (this->logging_parameters.size() != 0)
					this->logging_parameters.clear();
				for (size_t i = 0; i != parameters.size(); i++)
					this->logging_parameters[parameters_name[i]] = parameters[i];
			}

			// Only for python wrapper.
			// Todo make a subclass for this
			void add_dynamic_attribute(const std::vector<std::shared_ptr<double>>& attributes)
			{
				if (this->attr_per_run_name_value.size() != 0)
					this->attr_per_run_name_value.clear();
				for (size_t i = 0; i != attributes.size(); i++)
					this->attr_per_run_name_value["attr" + common::to_string(i + 1)] = attributes[i];
			}


			void add_dynamic_attribute(const std::vector<std::shared_ptr<double>>& attributes,
			                           const std::vector<std::string>& attributes_name)
			{
				if (attributes_name.size() != attributes.size())
					common::log::error("Attributes and their names are given with different size.");

				if (this->attr_per_run_name_value.size() != 0)
					this->attr_per_run_name_value.clear();
				for (size_t i = 0; i != attributes.size(); i++)
					this->attr_per_run_name_value[attributes_name[i]] = attributes[i];
			}

			void set_dynamic_attributes_name(const std::vector<std::string>& attributes_name)
			{
				if (this->attr_per_run_name_value.size() != 0)
					this->attr_per_run_name_value.clear();
				//default value set as -9999.
				for (size_t i = 0; i != attributes_name.size(); i++)
					this->attr_per_run_name_value[attributes_name[i]] = std::make_shared<double>(-9999);
			}

			void set_dynamic_attributes_name(const std::vector<std::string>& attributes_name,
			                                 const std::vector<double>& initial_attributes)
			{
				if (attributes_name.size() != initial_attributes.size())
					common::log::error("Attributes and their names are given with different size.");

				for (size_t i = 0; i != attributes_name.size(); i++)
					this->attr_per_run_name_value[attributes_name[i]] = std::make_shared<double>(initial_attributes[i]);
			}

			void set_dynamic_attributes(const std::vector<std::string>& attributes_name,
			                            const std::vector<double>& attributes)
			{
				if (attributes_name.size() != attributes.size())
					common::log::error("Attributes and their names are given with different size.");

				for (size_t i = 0; i != attributes_name.size(); ++i)
					if (this->attr_per_run_name_value.find(attributes_name[i]) != this->attr_per_run_name_value.end())
						*this->attr_per_run_name_value[attributes_name[i]] = attributes[i];
					else
						common::log::error("Dynamic attributes " + attributes_name[i] + " does not exist");
			}

			void set_parameters_name(const std::vector<std::string>& parameters_name)
			{
				if (this->logging_parameters.size() != 0)
					this->logging_parameters.clear();

				//default value set as -9999.
				for (size_t i = 0; i != parameters_name.size(); i++)
					this->logging_parameters[parameters_name[i]] = std::make_shared<double>(-9999);
			}

			void set_parameters_name(const std::vector<std::string>& parameters_name,
			                         const std::vector<double>& initial_parameters)
			{
				if (parameters_name.size() != initial_parameters.size())
					common::log::error("Parameters and their names are given with different size.");

				for (size_t i = 0; i != parameters_name.size(); i++)
					this->logging_parameters[parameters_name[i]] = std::make_shared<double>(initial_parameters[i]);
			}

			void set_parameters(const std::vector<std::string>& parameters_name, const std::vector<double>& parameters)
			{
				if (parameters_name.size() != parameters.size())
					common::log::error("Parameters and their names are given with different size.");

				for (size_t i = 0; i != parameters_name.size(); ++i)
					if (this->logging_parameters.find(parameters_name[i]) != this->logging_parameters.end())
						*this->logging_parameters[parameters_name[i]] = parameters[i];
					else
						common::log::error("Parameter " + parameters_name[i] + " does not exist");
			}
		};
	}
}
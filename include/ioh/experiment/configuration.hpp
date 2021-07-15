#pragma once

#include "ioh/common.hpp"

namespace ioh
{
    namespace experiment
    {
        /**
         * \brief A class to read configuration files.
         */
        class Configuration
        {
            /**
             * \brief The path of the configuration file. default: configuration.ini
             */
            fs::path config_file_ = "configuration.ini";

            /**
             * \brief The name of the suite
             */
            std::string suite_name_;

            /**
             * \brief A vector of problem id's to be included in the experiment
             */
            std::vector<int> problem_ids_;

            /**
             * \brief A vector of instance id's to be included in the experiment
             */
            std::vector<int> instances_;

            /**
             * \brief A vector of dimensions on which to test the problems & instances
             */
            std::vector<int> dimensions_;

            /**
             * \brief The root folder of the output directory in where data is stored
             */
            std::string output_directory_;

            /**
             * \brief The name of the subfolder in the output_directory where to store data
             */
            std::string result_folder_;

            /**
             * \brief An addition info string, to include in logging algorithm specific data. 
             */
            std::string algorithm_info_;

            /**
             * \brief The name of the algorithm
             */
            std::string algorithm_name_;

            /**
             * \brief `complete_triggers_` is the switch of `.cdat` files, which will store evaluations of all iterations.
             * Set complete_triggers as true if you want to output `.cdat` files.
             */
            bool complete_triggers_{};

            /**
             * \brief `update_triggers_` is the switch of `.dat` files, which will store evaluations if the best found
             * solution is updated. Set `update_triggers_` true if you want to output `.dat` files.
             */
            bool update_triggers_{};

            /**
             * \brief `number_interval_triggers_` configures the `.idat` files. `.idat` files log evaluations in a fixed
             *  frequency. `number_interval_triggers_` sets the value of the frequency. If you do not want to generate
             *  `.idat` files, set  `number_interval_triggers_` as 0.
             */
            int number_interval_triggers_{};

            /**
             * \brief `number_target_triggers_` is a value defines the number of evaluations to be logged between 10^i
             *  and 10^(i+1). If you do not want to generate `.tdat` files, set both \ref `number_target_triggers_`
             *  and \ref `base_evaluation_triggers_` as nullptr.
             */
            int number_target_triggers_{};

            /**
             * \brief `base_evaluation_triggers_` defines the base evaluations used to produce an additional
             * evaluation-based logging. For example, if `base_evaluation_triggers_` = {1,2,5}, the logger will be
             * triggered by evaluations dim*1, dim*2, dim*5, 10*dim*1, 10*dim*2, 10*dim*5, 100*dim*1, 100*dim*2,
             * 100*dim*5, ... . If you do not want to generate `.tdat` files, set  `base_evaluation_triggers_` as nullptr.
             */
            std::vector<int> base_evaluation_triggers_;

            /**
             * \brief The maximal number of problems to be included in the experiment
             */
            int max_number_of_problem_{};

            /**
             * \brief The maximal dimension on which to execute the experiments
             */
            int max_dimension_{};


            /**
             * \brief Used to hold experiment specific data in a convenient structure.
             */
            common::Container data_;
        public:

            /**
             * \brief Placeholder, used by experimenter when an experiment is not build from a
             * Configuration. 
             */
            Configuration() = default;

            /**
             * \brief Read a config file an constructs a Configuration instance
             * \param filename The path of the config file 
             */
            explicit Configuration(fs::path &filename) :
                config_file_(filename)
            {
                read_config();
            }

            /**
             * \brief Reads a config file and sets the parameters required to construct an
             * experiment. 
             */
            void read_config()
            {
                load();
                suite_name_ = data_.get("suite", "suite_name");

                // TODO: Make this meta data stuff nice in a struct somewhere
                if (suite_name_ == "BBOB")
                {
                    max_number_of_problem_ = 24;
                    max_dimension_ = 100;
                }
                else if (suite_name_ == "PBO")
                {
                    max_dimension_ = 20000;
                    max_number_of_problem_ = 25;
                }

                problem_ids_ = data_.get_int_vector("suite", "problem_id", 1,
                                                    max_number_of_problem_);
                instances_ = data_.get_int_vector(
                    "suite", "instance_id", 1, 100);
                dimensions_ = data_.get_int_vector(
                    "suite", "dimension", 1, max_dimension_);

                output_directory_ = data_.get("logger", "output_directory");
                result_folder_ = data_.get("logger", "result_folder");
                algorithm_info_ = data_.get("logger", "algorithm_info");
                algorithm_name_ = data_.get("logger", "algorithm_name");

                complete_triggers_ = data_.get_bool(
                    "observer", "complete_triggers");
                update_triggers_ = data_.
                    get_bool("observer", "update_triggers");
                base_evaluation_triggers_ = data_.get_int_vector(
                    "observer", "base_evaluation_triggers", 0, 10);
                number_target_triggers_ = data_.get_int("observer",
                                                        "number_target_triggers");
                number_interval_triggers_ = data_.get_int("observer",
                                                          "number_interval_triggers");
            }


            /**
             * \brief Loads the data stored in the \ref config_file_ into \ref data_
             */
            void load()
            {
                std::string line;
                auto fp = common::file::open_file(config_file_);

                char key[IOH_MAX_KEY_NUMBER];
                char value[IOH_MAX_KEY_NUMBER];
                char section[IOH_MAX_KEY_NUMBER];

                while (getline(fp, line))
                {
                    line = common::strip(line);
                    if (line.empty() || line.front() == '#' || line.front() == ';')
                        continue;
                    // TODO: check why we cannot use sccanf_s here, as this throws an access violation warning
                    if (line.front() == '[' && line.back() == ']')
                        sscanf(line.c_str(), "[%[^]]", section);
                    else if (
                        sscanf(line.c_str(), "%[^=] = \"%[^\"]", key, value) == 2
                        || sscanf(line.c_str(), "%[^=] = '%[^\']", key, value) == 2
                        || sscanf(line.c_str(), "%[^=] = %[^;#]", key, value) == 2)
                        data_.set(section, key, value);
                    else {
                        IOH_DBG(error, "Error in parsing .ini file on line: " << line);
                        assert(false);
                    }
                }
            }

            /**
             * \brief Get method for suite name
             * \return The private variable \ref suite_name_
             */
            [[nodiscard]] std::string suite_name() const
            {
                return suite_name_;
            }

            /**
             * \brief Get method for the problem id's included in the experiment
             * \return The private variable \ref problem_id_
             */
            [[nodiscard]] std::vector<int> problem_ids() const
            {
                return problem_ids_;
            }

            /**
             * \brief Get method for the instance id's included in the experiment
             * \return The private variable \ref instance_id_
             */
            [[nodiscard]] std::vector<int> instances() const
            {
                return instances_;
            }

            /**
             * \brief Get method for the dimensions tested in the experiment
             * \return The private variable \ref dimension_
            */
            [[nodiscard]] std::vector<int> dimensions() const
            {
                return dimensions_;
            }

            /**
             * \brief Get method for the root directory of the experiment's output
             * \return The private variable \ref output_directory_
             */
            [[nodiscard]] std::string output_directory() const
            {
                return output_directory_;
            }

            /**
             * \brief Get method for the name of the result folder for the experiment
             * \return The private variable \ref result_folder_
             */
            [[nodiscard]] std::string result_folder() const
            {
                return result_folder_;
            }

            /**
             * \brief Get method for the optional algorithm information string
             * \return The private variable \ref algorithm_info_
             */
            [[nodiscard]] std::string algorithm_info() const
            {
                return algorithm_info_;
            }

            /**
             * \brief Get method for the name of the algorithm used in the experiment
             * \return The private variable \ref algorithm_name_
             */
            [[nodiscard]] std::string algorithm_name() const
            {
                return algorithm_name_;
            }

            /**
             * \brief Get method for complete_triggers_
             * \return The private variable \ref complete_triggers_
             */
            [[nodiscard]] bool complete_triggers() const
            {
                return complete_triggers_;
            }

            /**
             * \brief Get method for update_triggers_
             * \return The private variable \ref update_triggers_
             */
            [[nodiscard]] bool update_triggers() const
            {
                return update_triggers_;
            }

            /**
             * \brief Get method for base_evaluation_triggers_
             * \return The private variable \ref base_evaluation_triggers_
             */
            [[nodiscard]] std::vector<int> base_evaluation_triggers() const
            {
                return base_evaluation_triggers_;
            }

            /**
             * \brief Get method for number_target_triggers_
             * \return The private variable \ref number_target_triggers_
             */
            [[nodiscard]] int number_target_triggers() const
            {
                return number_target_triggers_;
            }

            /**
             * \brief Get method for number_interval_triggers_
             * \return The private variable \ref number_interval_triggers_
             */
            [[nodiscard]] int number_interval_triggers() const
            {
                return number_interval_triggers_;
            }
        };
    }
}

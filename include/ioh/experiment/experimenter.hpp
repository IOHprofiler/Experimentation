#pragma once

#include "configuration.hpp"
#include "ioh/common.hpp"
#include "ioh/logger.hpp"
#include "ioh/suite.hpp"

namespace ioh {
    namespace experiment {
        /**
         * \brief A Class used for running experiments
         * \tparam ProblemType The type of the problems used in the experiments, needs to be a subclass of
         * \ref `ioh::problem::Problem`.
         */
        template <typename ProblemType>
        class Experimenter {
        public:
            /**
             * \brief A typedef for an optimizer; a function which takes a \ref ioh::problem::base and ioh::logger::csv
             * as arguments.
             */
            using Algorithm = std::function<void(std::shared_ptr<ProblemType>)>;

            Experimenter() = delete;
            Experimenter(const Experimenter &) = delete;
            Experimenter(const Experimenter &&) = delete;
            Experimenter &operator=(const Experimenter &) = delete;
            Experimenter &operator=(const Experimenter &&) = delete;
            ~Experimenter() = default;

            /**
             * \brief Constructs an Experimenter object, from a config file
             * \param config_file_name the path to the configuration file
             * \param algorithm a function pointer of type \ref algorithm_type
             */
            Experimenter(fs::path &config_file_name, Algorithm algorithm)
                : conf_(config_file_name), algorithm_(algorithm) {
                auto suite_name = conf_.suite_name();
                auto problems = conf_.problem_ids();
                auto instances = conf_.instances();
                auto dimensions = conf_.dimensions();

                suite_ = suite::SuiteRegistry<ProblemType>::instance().create(suite_name, problems, instances, dimensions);
                    
                if (suite_ == nullptr) {
                    IOH_DBG(error,"Creating suite fails, please check your Configuration");
                    assert(suite_ != nullptr);
                }

                logger_ = std::make_shared<logger::Default>(conf_);

                if (logger_ == nullptr) {
                    IOH_DBG(error,"Creating logger fails, please check your Configuration");
                    assert(logger_ != nullptr);
                }

            }

            /**
             * \brief Constructs an Experimenter object from a Suite object and an Logger object
             * \param suite A suite object
             * \param logger A csv logger object
             * \param algorithm a function pointer of type \ref algorithm_type
             * \param independent_runs the number of repetitions default = 1
             */
            Experimenter(std::shared_ptr<suite::Suite<ProblemType>> suite,
                         std::shared_ptr<Logger> logger,
                         Algorithm algorithm = nullptr,
                         const int independent_runs = 1
                )
                : suite_(std::move(suite)),
                  logger_(std::move(logger)),
                  algorithm_(algorithm),
                  independent_runs_(independent_runs) {
            }

            /**
             * \brief Runs the experiment; Evaluates \ref algorithm_ on all problems X instances X dimensions,
             * for N = independent_runs_ repeated runs.
             */
            void run() {
                // const auto t_timer = common::CpuTimer("Total ");
                // std::cout << *this << std::endl;

                suite_ ->attach_logger(*logger_); 
                for (const auto& p : *suite_)
                {
                    const auto p_timer = common::CpuTimer();
                    // std::cout << *p << std::endl;
                    for (auto count = 0; count < independent_runs_; ++count)
                    {
                        algorithm_(p);
                        p->reset();
                        // std::cout << "." << std::flush;
                    }
                }
                //logger_->flush();
            }

            /**
             * \brief Set's the number of independent runs to be used in \ref run
             * \param n The number of runs
             */
            void independent_runs(const int n) {
                this->independent_runs_ = n;
            }

            /**
             * \brief Get's the number of independent runs to be used in \ref run
             */
            [[nodiscard]] int independent_runs() const {
                return this->independent_runs_;
            }

            /**
             * \brief Get method for \ref suite_
             * \return Private \ref suite_
             */
            [[nodiscard]] std::shared_ptr<suite::Suite<ProblemType>> suite() const {
                return suite_;
            }

            /**
             * \brief Get method for \ref csv_logger_
             * \return Private \ref csv_logger_
             */
            [[nodiscard]] std::shared_ptr<Logger> logger() const {
                return logger_;
            }

            /**
             * \brief Prints a verbose representation of an Experimenter object to an output stream.
             * \param os The output stream
             * \param obj The Experimenter object
             * \return The output steam
             */
            friend std::ostream &operator<<(std::ostream &os, const Experimenter &obj) {
                return os << "Experiment"
                       << "\nSuite: " << obj.suite_->name()
                       << "\nProblem: " << common::vector_to_string(obj.suite_->problem_ids())
                       << "\nInstance: " << common::vector_to_string(obj.suite_->instances())
                       << "\nDimension: " << common::vector_to_string(obj.suite_->dimensions());
            }

        private:
            /**
             * \brief A configuration object. Only used when Experimenter is instantiated from a config file.
             */
            Configuration conf_;

            /**
             * \brief The benchmark suite used in the Experiment
             */
            std::shared_ptr<suite::Suite<ProblemType>> suite_;

            /**
             * \brief A pointer to the logger. 
             */
            std::shared_ptr<Logger> logger_;

            /**
             * \brief A function pointer of type \ref algorithm_type
             */
            Algorithm algorithm_;

            /**
             * \brief The number of independent runs to be performed
             */
            int independent_runs_ = 1;
        };
    }
}

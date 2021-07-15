#pragma once

#include "utils.hpp"
#include "ioh/logger.hpp"

namespace ioh
{
    namespace problem
    {
        template <typename T>
        class Problem
        {
        protected:
            MetaData meta_data_;
            Constraint<T> constraint_;
            State<T> state_;
            Solution<T> objective_;
            Logger *logger_{};
            logger::Info log_info_;

            [[nodiscard]]
            bool check_input_dimensions(const std::vector<T> &x)
            {
                if (x.empty())
                {
                    IOH_DBG(warning,"The solution is empty.");
                    return false;
                }
                if (x.size() != static_cast<size_t>(meta_data_.n_variables))
                {
                    IOH_DBG(warning,"The dimension of solution is incorrect.");
                    return false;
                }
                return true;
            }

            template <typename Integer = T>
            typename std::enable_if<std::is_integral<Integer>::value, bool>::type check_input(const std::vector<T> &x)
            {
                return check_input_dimensions(x);
            }

            template <typename Floating = T>
            typename std::enable_if<std::is_floating_point<Floating>::value, bool>::type check_input(
                const std::vector<T> &x)
            {
                if (!check_input_dimensions(x))
                    return false;

                if (common::all_finite(x))
                    return true;

                if (common::has_nan(x))
                {
                    IOH_DBG(warning,"The solution contains NaN.");
                    return false;
                }
                if (common::has_inf(x))
                {
                    IOH_DBG(warning,"The solution contains Inf.");
                    return false;
                }
                IOH_DBG(warning,"The solution contains invalid values.");
                return false;
            }

            [[nodiscard]]
            virtual double evaluate(const std::vector<T> &x) = 0;


            [[nodiscard]]
            virtual std::vector<T> transform_variables(std::vector<T> x)
            {
                return x;
            }

            [[nodiscard]]
            virtual double transform_objectives(const double y)
            {
                return y;
            }

        public:
            explicit Problem(MetaData meta_data, Constraint<T> constraint, Solution<T> objective) :
                meta_data_(std::move(meta_data)), constraint_(std::move(constraint)),
                objective_(std::move(objective))
            {
                state_ = State<T>({std::vector<T>(meta_data_.n_variables, std::numeric_limits<T>::signaling_NaN()),
                                meta_data_.initial_objective_value});

                constraint_.check_size(meta_data_.n_variables);

                log_info_.optimum = objective_.as_double();
                log_info_.current = state_.current.as_double();
            }

            explicit Problem(MetaData meta_data, Constraint<T> constraint = Constraint<T>()):
                Problem(meta_data, constraint, {
                            std::vector<T>(meta_data.n_variables, std::numeric_limits<T>::signaling_NaN()),
                            meta_data.optimization_type == common::OptimizationType::Minimization
                            ? -std::numeric_limits<double>::infinity()
                            : std::numeric_limits<double>::infinity()
                        })
            {
            }

            virtual ~Problem() = default;


            virtual void reset()
            {
                state_.reset();
                if (logger_ != nullptr) {
                    logger_->reset();
                    logger_->attach_problem(meta_data_);
                }
            }

            /**
             * \brief Update the current log info
             */
            virtual void update_log_info()
            {
                log_info_.evaluations = static_cast<size_t>(state_.evaluations);
                log_info_.raw_y_best = state_.current_best_internal.y;
                log_info_.transformed_y = state_.current.y;
                log_info_.transformed_y_best = state_.current_best.y;
                log_info_.current = state_.current.as_double();
            }

            [[nodiscard]]
            logger::Info &log_info()
            {
                return log_info_;
            }

            void attach_logger(Logger &logger)
            {
                logger_ = &logger;
                logger_->attach_problem(meta_data_);
            }

            void detach_logger()
            {
                if (logger_ != nullptr)
                    logger_->reset();
                logger_ = nullptr;
            }

            double operator()(const std::vector<T> &x)
            {
                if (!check_input(x))
                    return std::numeric_limits<double>::signaling_NaN();

                state_.current.x = x;
                state_.current_internal.x = transform_variables(x);
                state_.current_internal.y = evaluate(state_.current_internal.x);
                state_.current.y = transform_objectives(state_.current_internal.y);
                state_.update(meta_data_, objective_);
                if (logger_ != nullptr)
                {
                    update_log_info();
                    logger_->log(log_info());
                }
                return state_.current.y;
            }

            [[nodiscard]]
            MetaData meta_data() const
            {
                return meta_data_;
            }

            [[nodiscard]]
            Solution<T> objective() const
            {
                return objective_;
            }

            [[nodiscard]]
            State<T> state() const
            {
                return state_;
            }

            [[nodiscard]]
            Constraint<T> constraint() const
            {
                return constraint_;
            }

            friend std::ostream &operator<<(std::ostream &os, const Problem &obj)
            {
                return os
                    << "Problem(\n\t" << obj.meta_data_
                    << "\n\tconstraint: " << obj.constraint_
                    << "\n\tstate: " << obj.state_
                    << "\n\tobjective: " << obj.objective_ << "\n)";
            }
        };

        template <typename T>
        using Function = std::function<double(const std::vector<T> &)>;

        template <typename ProblemType>
        using ProblemRegistryType = common::RegisterWithFactory<ProblemType, int, int>;

        template <typename ProblemType>
        using ProblemFactoryType = common::Factory<ProblemType, int, int>;

        template <class Derived, class Parent>
        using AutomaticProblemRegistration = common::AutomaticTypeRegistration<Derived, ProblemRegistryType<Parent>>;

        template <class Parent>
        using ProblemRegistry = ProblemRegistryType<Parent>;

        template <typename T>
        class WrappedProblem final : public Problem<T>
        {
        protected:
            Function<T> function_;

            double evaluate(const std::vector<T> &x) override
            {
                return function_(x);
            }

        public:
            WrappedProblem(Function<T> f, const std::string &name, const int n_variables,
                           const common::OptimizationType optimization_type = common::OptimizationType::Minimization,
                           Constraint<T> constraint = Constraint<T>()
                ) :
                Problem<T>(MetaData(0, name, n_variables, optimization_type), constraint),
                function_(f)
            {
            }
        };

        template <typename T>
        WrappedProblem<T> wrap_function(Function<T> f, const std::string &name, const int n_variables = 5,
                                        const common::OptimizationType optimization_type =
                                            common::OptimizationType::Minimization,
                                        Constraint<T> constraint = Constraint<T>())
        {
            ProblemFactoryType<Problem<T>>::instance().include(name, 0, [=](const int, const int dimension)
            {
                return std::make_unique<WrappedProblem<T>>(f, name, dimension, optimization_type);
            });
            return WrappedProblem<T>{f, name, n_variables, optimization_type, constraint};
        }

        using Real = Problem<double>;
        using Integer = Problem<int>;

        template <typename ProblemType>
        class RealProblem : public Real, AutomaticProblemRegistration<ProblemType, Real>
        {
        public:
            using Real::Real;
        };

        template <typename ProblemType>
        struct IntegerProblem : Integer, AutomaticProblemRegistration<ProblemType, Integer>
        {
            using Integer::Integer;
        };
    }
}

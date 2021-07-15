#pragma once

#include "ioh/problem.hpp"

namespace ioh::suite
{
    template <typename ProblemType>
    class Suite
    {
    public:
        using Problem = std::shared_ptr<ProblemType>;
        using Factory = problem::ProblemFactoryType<ProblemType>;

        struct Iterator
        {
            using ValueType = typename std::vector<Problem>::value_type;
            using PointerType = ValueType *;
            using ReferenceType = ValueType &;
            Suite *suite;

            explicit Iterator(PointerType p, Suite *s, const bool track_problems = true):
                suite(s), ptr(p), begin(s->problems_.data()),
                end(s->problems_.data() + s->problems_.size()),
                track_problems(track_problems)
            {
                track_problem();
            }

            void track_problem() const
            {
                if (track_problems && ptr != end && suite->logger_ != nullptr)
                    (*ptr)->attach_logger(*suite->logger_);
            }

            Iterator &operator++()
            {
                ++ptr;
                track_problem();
                return *this;
            }

            Iterator &operator++(int)
            {
                Iterator it(*this);
                ++(*this);
                return it;
            }

            ReferenceType operator[](int index)
            {
                return *(ptr + index);
            }

            PointerType operator->()
            {
                return ptr;
            }

            ReferenceType operator*()
            {
                return *ptr;
            }

            bool operator==(const Iterator &other)
            {
                return ptr == other.ptr;
            }

            bool operator!=(const Iterator &other)
            {
                return !(*this == other);
            }

        private:
            PointerType ptr;
            PointerType begin;
            PointerType end;
            bool track_problems;
        };

    private:
        std::string name_;
        std::vector<Problem> problems_;
        std::vector<int> problem_ids_;
        std::vector<int> instances_;
        std::vector<int> dimensions_;
        Logger *logger_{};

        [[nodiscard]]
        int check_parameter(const int parameter, const int ub, const int lb = 1) const
        {
            if (parameter < lb || parameter > ub) {
                IOH_DBG(error,"Parameter value " << parameter << " is out of bounds");
                assert(lb <= parameter and parameter <= ub);
            }
            return parameter;
        }

    public:
        Suite(const std::vector<int> &problem_ids, const std::vector<int> &instances,
              const std::vector<int> &dimensions, const std::string &name,
              const int max_instance = 1000, const int max_dimension = 1000, Factory &factory = Factory::instance()
            ) :
            name_(name), problems_(), problem_ids_(problem_ids), instances_(instances), dimensions_(dimensions)
        {
            const auto available_ids = factory.ids();
            const int max_problem_id = *std::max_element(available_ids.begin(), available_ids.end());
            const int min_problem_id = *std::min_element(available_ids.begin(), available_ids.end());

            for (const auto &problem_id : problem_ids)
                for (const auto &n_variables : dimensions)
                    for (const auto &instance : instances)
                        problems_.emplace_back(factory.create(
                            check_parameter(problem_id, max_problem_id, min_problem_id),
                            check_parameter(instance, max_instance),
                            check_parameter(n_variables, max_dimension)
                            ));
        }

        virtual ~Suite() = default;

        void reset()
        {
            if (logger_ != nullptr)
                logger_->reset();
            for (auto &problem : problems_)
                problem.reset();
        }

        void attach_logger(Logger &logger)
        {
            logger_ = &logger;
            logger_->attach_suite(name());
        }

        void detach_logger()
        {
            if (logger_ != nullptr)
                logger_->reset();
            logger_ = nullptr;
        }

        [[nodiscard]]
        Iterator begin(const bool track_problems = true)
        {
            return Iterator(problems_.data(), this, track_problems);
        }

        [[nodiscard]]
        Iterator end()
        {
            return Iterator(problems_.data() + problems_.size(), this);
        }

        [[nodiscard]]
        std::vector<int> problem_ids() const
        {
            return problem_ids_;
        }

        [[nodiscard]]
        std::vector<int> dimensions() const
        {
            return dimensions_;
        }

        [[nodiscard]]
        std::vector<int> instances() const
        {
            return instances_;
        }

        [[nodiscard]]
        std::string name() const
        {
            return name_;
        }

        [[nodiscard]]
        size_t size() const
        {
            return problem_ids_.size() * instances_.size() * dimensions_.size();
        }
    };

    template <typename ProblemType>
    using SuiteRegistryType = common::RegisterWithFactory<
        Suite<ProblemType>, std::vector<int>, std::vector<int>, std::vector<int>>;


    template <typename ProblemType>
    using SuiteFactoryType = common::Factory<
        Suite<ProblemType>, std::vector<int>, std::vector<int>, std::vector<int>>;


    template <class Derived, class ProblemType>
    using AutomaticSuiteRegistration = common::AutomaticTypeRegistration<Derived, SuiteRegistryType<ProblemType>>;

    template <class ProblemType>
    using SuiteRegistry = SuiteRegistryType<ProblemType>;


    template <class Derived>
    struct RealSuite : Suite<problem::Real>, AutomaticSuiteRegistration<Derived, problem::Real>
    {
        using Suite<problem::Real>::Suite;
    };


    template <class Derived>
    struct IntegerSuite : Suite<problem::Integer>, AutomaticSuiteRegistration<Derived, problem::Integer>
    {
        using Suite<problem::Integer>::Suite;
    };

    ////////////////////// Available Suites //////////////////////
    struct Real final : RealSuite<Real>
    {
        Real(const std::vector<int> &problem_ids, const std::vector<int> &instances,
             const std::vector<int> &dimensions) :
            RealSuite(problem_ids, instances, dimensions, "Real")
        {
        }
    };

    struct Integer final : IntegerSuite<Integer>
    {
        Integer(const std::vector<int> &problem_ids, const std::vector<int> &instances,
                const std::vector<int> &dimensions) :
            IntegerSuite(problem_ids, instances, dimensions, "Integer")
        {
        }
    };

    struct BBOB final : RealSuite<BBOB>
    {
        BBOB(const std::vector<int> &problem_ids, const std::vector<int> &instances,
             const std::vector<int> &dimensions) :
            RealSuite(problem_ids, instances, dimensions, "BBOB", 100, 100,
                      reinterpret_cast<Factory &>(problem::ProblemFactoryType<problem::BBOB>::instance())
                )
        {
        }
    };

    struct PBO final : IntegerSuite<PBO>
    {
        PBO(const std::vector<int> &problem_ids, const std::vector<int> &instances,
            const std::vector<int> &dimensions) :
            IntegerSuite(problem_ids, instances, dimensions, "PBO", 100, 20000,
                         reinterpret_cast<Factory &>(problem::ProblemFactoryType<problem::PBO>::instance()))
        {
        }
    };
}

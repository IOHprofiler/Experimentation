#pragma once

#include "bbob_problem.hpp"

namespace ioh::problem::bbob
{
    class SharpRidge final : public BBOProblem<SharpRidge>

    {
        int n_linear_dimensions_;
    protected:
        double evaluate(const std::vector<double> &x) override
        {
            static const auto alpha = 100.0;

            auto result = 0.0;
            for (auto i = n_linear_dimensions_; i < meta_data_.n_variables; ++i)
                result += x.at(i) * x.at(i);

            result = alpha * sqrt(result / n_linear_dimensions_);
            for (auto i = 0; i < n_linear_dimensions_; ++i)
                result += x.at(i) * x.at(i) / n_linear_dimensions_;

            return result;
        }

        std::vector<double> transform_variables(std::vector<double> x) override
        {
            using namespace transformation::variables;
            subtract(x, objective_.x);
            affine(x, transformation_state_.second_transformation_matrix, transformation_state_.transformation_base);
            return x;
        }

    public:
        SharpRidge(const int instance, const int n_variables) :
            BBOProblem(13, instance, n_variables, "SharpRidge"),
        n_linear_dimensions_(static_cast<int>(
            ceil(meta_data_.n_variables <= 40 ? 1 : meta_data_.n_variables / 40.0)))
        {
        }
    };
}

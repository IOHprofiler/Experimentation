#pragma once

#include "bbob_problem.hpp"

namespace ioh::problem::bbob
{
    template <typename T>
    class Gallagher : public BBOProblem<T>
    {
        struct Peak
        {
            double value;
            std::vector<double> scales;

            struct Permutation
            {
                double value;
                int index;

                bool operator<(const Permutation &b) const
                {
                    return value < b.value;
                }

                static std::vector<Permutation> sorted(const int n, const int seed)
                {
                    const auto random_numbers = common::random::bbob2009::uniform(n, seed);
                    std::vector<Permutation> permutations(n);

                    for (auto i = 0; i < n; ++i)
                        permutations[i] = {random_numbers.at(i), i};

                    std::sort(permutations.begin(), permutations.end());
                    return permutations;
                }
            };

            Peak(const double value, const int seed, const int n_variables, const double condition) :
                value(value), scales(n_variables)
            {
                auto permutations = Permutation::sorted(n_variables, seed);
                for (auto i = 0; i < n_variables; ++i)
                    scales[i] = pow(condition,
                                    static_cast<double>(permutations[i].index) / (static_cast<double>(n_variables) - 1.)
                                    - 0.5);
            }

            static std::vector<Peak> get_peaks(const int n, const int n_variables, const int seed, const double max_condition)
            {
                static const auto f0 = 1.1, f1 = 9.1, mc = 1000.;
                static const auto divisor = static_cast<double>(n) - 2.;

                auto permutations = Permutation::sorted(n - 1, seed);

                std::vector<Peak> peaks(1, {10.0, seed, n_variables, max_condition});
                for (auto i = 1; i < n; ++i)
                    peaks.emplace_back(static_cast<double>(i - 1) / divisor * (f1 - f0) + f0, seed + (1000 * i),
                                       n_variables,
                                       pow(mc, static_cast<double>(permutations[i - 1].index) / divisor));

                return peaks;
            }
        };

        std::vector<std::vector<double>> x_transformation_;
        std::vector<Peak> peaks_;
        double factor_;

    protected:
        double evaluate(const std::vector<double> &x) override
        {
            static const auto a = 0.1;
            std::vector<double> x_transformed(this->meta_data_.n_variables);
            auto penalty = 0.;

            for (auto i = 0; i < this->meta_data_.n_variables; i++)
            {
                const auto out_of_bounds = fabs(x.at(i)) - 5.;
                if (out_of_bounds > 0.)
                    penalty += out_of_bounds * out_of_bounds;

                x_transformed[i] = std::inner_product(x.begin(), x.end(),
                                                      this->transformation_state_.second_rotation.at(i).begin(), 0.0);
            }
#if defined(__GNUC__)
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wsequence-point"
#endif
            auto result = 10. - std::accumulate(
                peaks_.begin(), peaks_.end(), 0.0,
                [&, i = 0](const double sum, const Peak &peak) mutable
                {
                    const auto z = std::accumulate(
                        x_transformed.begin(), x_transformed.end(), 0.0,
                        [&, j = 0](const double lhs, const double rhs) mutable
                        {
                            return lhs + peaks_.at(i).scales.at(j) * pow(rhs - x_transformation_.at(j++).at(i), 2.0);
                        });
                    i++;
                    return std::max(sum, peak.value * exp(factor_ * z));
                });
#if defined(__GNUC__)
            #pragma GCC diagnostic pop
#endif

            if (result > 0)
            {
                result = std::log(result) / a;
                result = pow(exp(result + 0.49 * (sin(result) + sin(0.79 * result))), a);
            }
            else if (result < 0)
            {
                result = std::log(-result) / a;
                result = -pow(exp(result + 0.49 * (sin(0.55 * result) + sin(0.31 * result))), a);
            }

            return result * result + penalty;
        }

    public:
        Gallagher(const int problem_id, const int instance, const int n_variables, const std::string &name,
                  const int number_of_peaks, const double b = 10., const double c = 5.0,
                  double max_condition = sqrt(1000.)) :
            BBOProblem<T>(problem_id, instance, n_variables, name),
            x_transformation_(n_variables, std::vector<double>(number_of_peaks)),
            peaks_(Peak::get_peaks(number_of_peaks, n_variables, this->transformation_state_.seed, max_condition)),
            factor_(-0.5 / static_cast<double>(n_variables))
        {
            const auto random_numbers = common::random::bbob2009::uniform(
                this->meta_data_.n_variables * number_of_peaks, this->transformation_state_.seed);


            for (auto i = 0; i < this->meta_data_.n_variables; ++i)
            {
                this->objective_.x[i] = 0.8 * (b * random_numbers[i] - c);
                for (auto j = 0; j < number_of_peaks; ++j)
                {
                    for (auto k = 0; k < this->meta_data_.n_variables; ++k)
                        x_transformation_[i][j] += this->transformation_state_.second_rotation[i][k] * (
                            b * random_numbers.at(j * this->meta_data_.n_variables + k) - c
                        );
                    if (j == 0)
                        x_transformation_[i][j] *= 0.8;
                }
            }
        }
    };

    class Gallagher101 final : public Gallagher<Gallagher101>
    {
    public:
        Gallagher101(const int instance, const int n_variables):
            Gallagher(21, instance, n_variables, "Gallagher101", 101, 10., 5.0)
        {
        }
    };
}

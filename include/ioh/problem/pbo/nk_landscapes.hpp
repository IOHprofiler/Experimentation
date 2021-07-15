#pragma once
#include "pbo_problem.hpp"

namespace ioh
{
    namespace problem
    {
        namespace pbo
        {
            class NKLandscapes final: public PBOProblem<NKLandscapes>
            {
            protected:
                std::vector<std::vector<double>> f_;
                std::vector<std::vector<int>> e_;
                int k_ = 1;

                void set_n_k(const int n, const int k)
                {
                    e_.clear();
                    f_.clear();
                    k_ = k;
                    if (k > n)
                    {
                        IOH_DBG(error,"NK_Landscapes, k > n");
                        assert(k<=n);
                    }
                    for (auto i = 0; i != n; ++i)
                    {
                        const auto rand_vec = common::random::uniform(static_cast<size_t>(k), static_cast<long>(k * (i + 1)));

                        std::vector<int> sampled_number;
                        std::vector<int> population;
                        for (auto j = 0; j < n; ++j)
                        {
                            population.push_back(j);
                        }

                        for (auto i1 = n - 1; i1 > 0; --i1)
                        {
                            const auto rand_pos = static_cast<int>(floor(rand_vec[n - 1 - i1] * (i1 + 1)));
                            const auto temp = population[i1];
                            population[i1] = population[rand_pos];
                            population[rand_pos] = temp;
                            sampled_number.push_back(population[i1]);
                            if (n - i1 - 1 == k - 1)
                            {
                                break;
                            }
                        }
                        if (n == k)
                        {
                            sampled_number.push_back(population[0]);
                        }
                        e_.push_back(sampled_number);
                    }
                    for (auto i = 0; i != n; ++i)
                    {
                        f_.emplace_back(common::random::uniform(static_cast<size_t>(pow(2, k + 1)),
                                                             static_cast<long>(k * (i + 1) * 2)));
                    }
                }

                double evaluate(const std::vector<int> &x) override
                {
                    auto result = 0.0;
                    for (auto i = 0; i != meta_data_.n_variables; ++i)
                    {
                        size_t index = x[i];
                        for (auto j = 0; j != k_; ++j)
                        {
                            index = index + static_cast<size_t>(pow(2, j + 1) * x[e_[i][j]]);
                        }
                        result += f_[i][index];
                    }

                    result = result / static_cast<double>(meta_data_.n_variables);
                    return -result;
                }

            public:
                /**
                 * \brief Construct a new NK_Landscapes object. Definition refers to
                 *https://doi.org/10.1007/978-3-030-58115-2_49
                 *
                 * \param instance The instance number of a problem, which controls the transformation
                 * performed on the original problem.
                 * \param n_variables The dimensionality of the problem to created, 4 by default.
                 **/
                NKLandscapes(const int instance, const int n_variables) :
                    PBOProblem(25, instance, n_variables, "NKLandscapes")
                {
                    set_n_k(n_variables, k_);
                }
            };
        } // namespace pbo
    } // namespace problem
} // namespace ioh

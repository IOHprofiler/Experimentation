#pragma once
#include "pbo_problem.hpp"


namespace ioh
{
    namespace problem
    {
        namespace pbo
        {
            class NQueens final: public PBOProblem<NQueens>
            {
            protected:
                double evaluate(const std::vector<int> &x) override
                {
                    int j, i;
                    const auto n_queens = static_cast<int>(sqrt(static_cast<double>(meta_data_.n_variables)) + 0.5);
                    auto number_of_queens_on_board = 0;
                    auto k_penalty = 0.0;
                    auto l_penalty = 0.0;
                    auto rows_penalty = 0.0;
                    auto columns_penalty = 0.0;
                    auto index = 0;
                    const auto c = static_cast<float>(n_queens);

                    if (floor(sqrt(static_cast<double>(meta_data_.n_variables))) != sqrt(static_cast<double>(meta_data_.n_variables))) {
                        IOH_DBG(error,"Number of parameters in the N Queen problem must be a square number");
                        assert(floor(sqrt(static_cast<double>(meta_data_.n_variables))) == sqrt(static_cast<double>(meta_data_.n_variables)));
                    }

                    for (; index < meta_data_.n_variables; index++)
                    {
                        if (x[index] == 1)
                        {
                            number_of_queens_on_board += 1;
                        }
                    }

                    for (j = 1; j <= n_queens; j++)
                    {
                        auto sum_column = 0.0;
                        for (i = 1; i <= n_queens; i++)
                        {
                            index = (i - 1) * n_queens + (j - 1) % n_queens;
                            sum_column += static_cast<double>(x[index]);
                        }
                        columns_penalty += std::max(0.0, -1.0 + sum_column);
                    }

                    for (i = 1; i <= n_queens; i++)
                    {
                        auto sum_raw = 0.0;
                        /*double sum_k = 0.0;
                        double sum_l = 0.0;*/
                        for (j = 1; j <= n_queens; j++)
                        {
                            index = (i - 1) * n_queens + (j - 1) % n_queens;
                            sum_raw += static_cast<double>(x[index]);
                        }
                        rows_penalty += std::max(0.0, -1.0 + sum_raw);
                    }

                    for (auto k = 2 - n_queens; k <= n_queens - 2; k++)
                    {
                        auto sum_k = 0.0;
                        for (i = 1; i <= n_queens; i++)
                        {
                            if (k + i >= 1 && k + i <= n_queens)
                            {
                                index = (i - 1) * n_queens + (k + i - 1) % n_queens;
                                sum_k += static_cast<double>(x[index]);
                            }
                        }
                        k_penalty += std::max(0.0, -1.0 + sum_k);
                    }
                    for (auto l = 3; l <= 2 * n_queens - 1; l++)
                    {
                        auto sum_l = 0.0;
                        for (i = 1; i <= n_queens; i++)
                        {
                            if (l - i >= 1 && l - i <= n_queens)
                            {
                                index = (i - 1) * n_queens + (l - i - 1) % n_queens;
                                sum_l += static_cast<double>(x[index]);
                            }
                        }
                        l_penalty += std::max(0.0, -1.0 + sum_l);
                    }
                    const auto result = static_cast<double>(number_of_queens_on_board - c * rows_penalty -
                                                            c * columns_penalty - c * k_penalty - c * l_penalty);
                    return static_cast<double>(result);
                }

            public:
                /**
                 * \brief Construct a new NQueens object. Definition refers to
                 *https://doi.org/10.1016/j.asoc.2019.106027
                 *
                 * \param instance The instance number of a problem, which controls the transformation
                 * performed on the original problem.
                 * \param n_variables The dimensionality of the problem to created, 4 by default.
                 **/
                NQueens(const int instance, const int n_variables) : 
                PBOProblem(23, instance, n_variables, "NQueens") 
                {
                    assert(sqrt(n_variables) - floor(sqrt(n_variables)) == 0);
                    objective_.y = {sqrt(n_variables)};
                }
            };
        } // namespace pbo
    } // namespace problem
} // namespace ioh

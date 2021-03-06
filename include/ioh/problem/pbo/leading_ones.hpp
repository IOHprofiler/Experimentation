#pragma once
#include "pbo_problem.hpp"

namespace ioh
{
    namespace problem
    {
        namespace pbo
        {
            class LeadingOnes final: public PBOProblem<LeadingOnes>
            {
            protected:
                double evaluate(const std::vector<int> &x) override
                {
                    auto result = 0.0;
                    for (auto i = 0; i < meta_data_.n_variables; ++i)  
                        if (x[i] == 1)
                            result = i + 1;
                        else
                            break;
                    
                    return result;
                }

            public:
                /**
                 * \brief Construct a new LeadingOnes object. Definition refers to
                 *https://doi.org/10.1016/j.asoc.2019.106027
                 *
                 * \param instance The instance number of a problem, which controls the transformation
                 * performed on the original problem.
                 * \param n_variables The dimensionality of the problem to created, 4 by default.
                 **/
                LeadingOnes(const int instance, const int n_variables) :
                    PBOProblem(2, instance, n_variables, "LeadingOnes")
                {
                    objective_.x = std::vector<int>(n_variables,1);
                    objective_.y = evaluate(objective_.x);
                }
            };
        } // namespace pbo
    } // namespace problem
} // namespace ioh

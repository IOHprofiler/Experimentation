#pragma once
#include "pbo_problem.hpp"

namespace ioh
{
    namespace problem
    {
        namespace pbo
        {
            class IsingTorus final : public PBOProblem<IsingTorus>
            {
            protected:
                static int modulo_ising_torus(const int x, const int n) { return (x % n + n) % n; }

                double evaluate(const std::vector<int> &x) override
                {
                    auto result = 0.0;
                    int neighbors[2];
                    const auto double_n = static_cast<double>(meta_data_.n_variables);
                    const auto lattice_size = static_cast<int>(sqrt(double_n));

                    if (floor(sqrt(double_n)) != sqrt(double_n))
                    {
                        IOH_DBG(error,"Number of parameters in the Ising square problem must be a square number");
                        assert(floor(sqrt(double_n)) == sqrt(double_n));
                    }

                    for (auto i = 0; i < lattice_size; ++i)
                    {
                        for (auto j = 0; j < lattice_size; ++j)
                        {
                            neighbors[0] = x[modulo_ising_torus(i + 1, lattice_size) * lattice_size + j];
                            neighbors[1] = x[lattice_size * i + modulo_ising_torus(j + 1, lattice_size)];
                            for (const auto neighbor : neighbors)
                                result += x[lattice_size * i + j] * neighbor +
                                    (1 - x[i * lattice_size + j]) * (1 - neighbor);
                        }
                    }
                    return result;
                }

            public:
                /**
                 * \brief Construct a new Ising_Torus object. Definition refers to
                 *https://doi.org/10.1016/j.asoc.2019.106027
                 *
                 * \param instance The instance number of a problem, which controls the transformation
                 * performed on the original problem.
                 * \param n_variables The dimensionality of the problem to created, 4 by default.
                 **/
                IsingTorus(const int instance, const int n_variables) :
                    PBOProblem(20, instance, n_variables, "IsingTorus")
                {
                    objective_.x = std::vector<int>(n_variables,1);
                    objective_.y = IsingTorus::evaluate(objective_.x);
                }
            };
        } // namespace pbo
    } // namespace problem
} // namespace ioh

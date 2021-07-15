#include <vector>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include "ioh.hpp"

#include "../utils.hpp" 


TEST(PBOfitness, dimension16)
{
    auto& ioh_dbg = clutchlog::logger();
    ioh_dbg.threshold(clutchlog::level::xdebug);

    const auto file_name = find_test_file("pbofitness16.in");

    std::string s;
    const auto &problem_factory = ioh::problem::ProblemRegistry<ioh::problem::PBO>::instance();

    std::ifstream infile(file_name.c_str());
    ASSERT_TRUE(infile.is_open());
    while (getline(infile, s))
    {
        auto tmp = split(s, " ");
        auto func_id = stoi(tmp[0]);
        auto ins_id = stoi(tmp[1]);
        auto x = string_to_vector_int(tmp[2]);
        auto f = stod(tmp[3]);

        auto problem = problem_factory.create(func_id, ins_id, 16);
        auto y = (*problem)(x);

        EXPECT_LE(abs(f - y), 0.0001) << "The fitness of function " << func_id << "( ins " << ins_id << " ) is " << f <<
 " ( not " << y << ").";
    }
}

TEST(PBOfitness, dimension100)
{
    auto& ioh_dbg = clutchlog::logger();
    ioh_dbg.threshold(clutchlog::level::xdebug);

    const auto file_name = find_test_file("pbofitness100.in");
    std::string s;
    const auto &problem_factory = ioh::problem::ProblemRegistry<ioh::problem::PBO>::instance();
    std::ifstream infile(file_name.c_str());
    ASSERT_TRUE(infile.is_open());
    while (getline(infile, s))
    {
        auto tmp = split(s, " ");
        auto func_id = stoi(tmp[0]);
        auto ins_id = stoi(tmp[1]);
        auto x = string_to_vector_int(tmp[2]);
        auto f = stod(tmp[3]);

        auto problem = problem_factory.create(func_id, ins_id, 100);
        auto y = (*problem)(x);
        EXPECT_LE(abs(f - y), 0.0001) << "The fitness of function " << func_id << "( ins " << ins_id << " ) is " << f <<
 " ( not " << y << ").";
    }
}

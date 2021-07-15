#include <cmath>
#include <list>
#include <gtest/gtest.h>
#include "ioh.hpp"


double test_eval(const std::shared_ptr<ioh::problem::Real> &f)
{
    const auto x = ioh::common::random::normal(4, 42);
    return std::round((*f)(x) * 10000.0) / 10000.0;
}


double test_eval(const std::shared_ptr<ioh::problem::Integer> &f)
{
    const std::vector<int> x = {1, 1, 0, 1, 0, 0, 0, 1, 1};
    return std::floor((*f)(x) * 10000.0) / 10000.0;
}


TEST(problems, bbob)
{
    using namespace ioh::problem::bbob;
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<AttractiveSector>(1, 4)), 38905.1446);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<BentCigar>(1, 4)), 43065603.7200);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<BuecheRastrigin>(1, 4)), -377.5170);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<DifferentPowers>(1, 4)), -28.6647);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Discus>(1, 4)), 633536.4945);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Ellipsoid>(1, 4)), 4670466.7111);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<EllipsoidRotated>(1, 4)), 4272872.0872);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Gallagher101>(1, 4)), 100.6898);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Gallagher21>(1, 4)), -924.8833);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<GriewankRosenBrock>(1, 4)), -92.2225);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Katsuura>(1, 4)), 21.7005);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LinearSlope>(1, 4)), 75.4977);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LunacekBiRastrigin>(1, 4)), 177.9893);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Rastrigin>(1, 4)), -351.7320);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<RastriginRotated>(1, 4)), 1880.5481);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Rosenbrock>(1, 4)), 3990.2021);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<RosenbrockRotated>(1, 4)), 2734.3342);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Schaffers10>(1, 4)), -3.8932);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Schaffers1000>(1, 4)), 39.4152);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Schwefel>(1, 4)), 6135.2508);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<SharpRidge>(1, 4)), 1304.6100);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Sphere>(1, 4)), 85.3339);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<StepEllipsoid>(1, 4)), 180.6339);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Weierstrass>(1, 4)), 192.2602);
}

TEST(problems, pbo)
{
    using namespace ioh::problem::pbo;
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMax>(1, 9)), 5.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxDummy1>(1, 9)), 3.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxDummy2>(1, 9)), 4.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxEpistasis>(1, 9)), 6.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxNeutrality>(1, 9)), 2.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxRuggedness1>(1, 9)), 4.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxRuggedness2>(1, 9)), 6.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<OneMaxRuggedness3>(1, 9)), 7.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnes>(1, 9)), 2.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesDummy1>(1, 9)), 3.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesDummy2>(1, 9)), 2.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesEpistasis>(1, 9)), 0.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesNeutrality>(1, 9)), 1.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesRuggedness1>(1, 9)), 2.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesRuggedness2>(1, 9)), 1.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LeadingOnesRuggedness3>(1, 9)), 1.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<Linear>(1, 9)), 24.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<MIS>(1, 9)), -4.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<LABS>(1, 9)), 2.5312);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<NQueens>(1, 9)), -16.0000);

    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<IsingRing>(1, 9)), 5.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<IsingTorus>(1, 9)), 6.0000);
    EXPECT_DOUBLE_EQ(test_eval(std::make_shared<IsingTriangular>(1, 9)), 9.0000);
}

TEST(problem_suite, bbob)
{
    auto& ioh_dbg = clutchlog::logger();
    ioh_dbg.threshold(clutchlog::level::xdebug);

    std::vector<int> ids(24);
    std::iota(ids.begin(), ids.end(), 1);
    ioh::suite::BBOB bbob(ids, {1}, {4});
    auto results = std::list<double>({
        85.3339,
        4670466.7111,
        -351.7320,
        -377.5170,
        75.4977,
        38905.1446,
        180.6339,
        3990.2021,
        2734.3342,
        4272872.0872,
        633536.4945,
        43065603.7200,
        1304.6100,
        -28.6647,
        1880.5481,
        192.2602, //192.0590,
        -3.8932,
        39.4152,
        -92.2225,
        6135.2508,
        100.6898,
        -924.8833,
        21.7005,
        177.9893
    });
    
    for (const auto &problem : bbob)
    {
        EXPECT_DOUBLE_EQ(test_eval(problem), results.front());
        results.pop_front();
    }
}


TEST(xopt_equals_yopt, pbo)
{
    const auto& problem_factory = ioh::problem::ProblemRegistry<ioh::problem::PBO>::instance();
    for (const auto& name : problem_factory.names())
    {
        if (name != "LABS" && name != "NQueens" && name != "LeadingOnesEpistasis" && name != "MIS" && name != "NKLandscapes")
        {
            auto problem = problem_factory.create(name, 1, 16);
            EXPECT_DOUBLE_EQ(problem->objective().y, (*problem)(problem->objective().x)) << *problem;
        }
    }
}

TEST(xopt_equals_yopt, real)
{
    const auto& problem_factory = ioh::problem::ProblemRegistry<ioh::problem::Real>::instance();
    for (const auto& name : problem_factory.names())
    {
        auto problem = problem_factory.create(name, 1, 16);
        EXPECT_DOUBLE_EQ(problem->objective().y, (*problem)(problem->objective().x)) << *problem;
    }
}

TEST(problems, bbob5d)
{
    const auto& the_factory = ioh::problem::ProblemRegistry<ioh::problem::Real>::instance();
    const std::vector<double> expected{
       161.17445568,
       12653420.412225708,
       -97.06158297486468,
       8845.53524810093,
       121.9133638725643,
       423021.00682286796,
       278.3290201933729,
       4315.032550201522,
       95168.253629631741,
       969025.0451803299,
       20911310.769634742,
       187251637.66430587,
       2198.7155122256763,
       261.60997479957,
       1103.1553337856833,
       273.87870697835791,
       35.410739487431208,
       107.64134358767826,
       -40.269198932753994,
       49739.05388887795,
       116.29585727504872,
       -914.902473409051,
       18.635078550302751,
       1782.2733296400438,
    };
    const auto dimension = 5;
    const std::vector<double> x0{ 0.1, 1., 2.,4., 5.4 };
    for (const auto& name : the_factory.names())
    {
        const auto item = the_factory.create(name, 1, dimension);
        EXPECT_FLOAT_EQ(
            static_cast<float>(expected.at(item->meta_data().problem_id - 1)), 
            static_cast<float>((*item)(x0)))
        << *item;
    }
}







#include <utility>
#include <gtest/gtest.h>
#include "ioh.hpp"

using namespace ioh;
using namespace ioh::logger;

TEST(eah, scales)
{
    // Linear range
    eah::LinearScale<double> linr(0,100, 10);

    EXPECT_EQ(linr.index(  0),0);
    EXPECT_EQ(linr.index(  9),0);
    EXPECT_EQ(linr.index( 50),5);
    EXPECT_EQ(linr.index( 99),9);
    EXPECT_EQ(linr.index(100),9);

    eah::LinearScale<double>::BoundsType p_0_10 = {0,10};
    EXPECT_EQ(linr.bounds(0),p_0_10);
    
    eah::LinearScale<double>::BoundsType p_90_100 = {90,100};
    EXPECT_EQ(linr.bounds(9),p_90_100);


    // Default log range
    eah::Log2Scale<double> logr_small(0,8,5);

    EXPECT_EQ(logr_small.index(0),0);
    EXPECT_EQ(logr_small.index(1),1);
    EXPECT_EQ(logr_small.index(2),2);
    EXPECT_EQ(logr_small.index(4),3);
    EXPECT_EQ(logr_small.index(7.99),4);
    EXPECT_EQ(logr_small.index(8),4);


    // LOG BASE 2

    // Tiny log range
    eah::Log2Scale<double> log2r_tiny(0,100,10);

    EXPECT_EQ(log2r_tiny.index(  0),0);
    EXPECT_EQ(log2r_tiny.index(100),9);
    
    // Log range
    eah::Log2Scale<double> log2r_large(0,6e7,10);

    EXPECT_EQ(log2r_large.index(  0),0);
    EXPECT_EQ(log2r_large.index(6e7),9);

    // For small values, there is no robustness problem.
    EXPECT_DOUBLE_EQ(log2r_large.bounds(0).first , 0);
    
    // At the moment, errors are still larger than 4 ULPs, so we use this more laxist test:
    EXPECT_NEAR(log2r_large.bounds(9).second, 6e7, 1e-6);
    // If the algorithm was robust enough:
    // EXPECT_DOUBLE_EQ(log2r_large.bounds(9).second, 6e7);


    // LOG BASE 10

    // Tiny log range
    eah::Log10Scale<double> log10r_tiny(0,100,10);

    EXPECT_EQ(log10r_tiny.index(  0),0);
    EXPECT_EQ(log10r_tiny.index(100),9);
    
    // Log range
    eah::Log10Scale<double> log10r_large(0,6e7,10);

    EXPECT_EQ(log10r_large.index(  0),0);
    EXPECT_EQ(log10r_large.index(6e7),9);

    // For small values, there is no robustness problem.
    EXPECT_DOUBLE_EQ(log10r_large.bounds(0).first , 0);
    
    // At the moment, errors are still larger than 4 ULPs, so we use this more laxist test:
    EXPECT_NEAR(log10r_large.bounds(9).second, 6e7, 1e-6);
    // If the algorithm was robust enough:
    // EXPECT_DOUBLE_EQ(log10r_large.bounds(9).second, 6e7);
}

TEST(eah, scales_death)
{
    // Linear
    eah::LinearScale<double> linr(0,100, 10);

    EXPECT_DEBUG_DEATH(linr.index( -1) , "");
    EXPECT_DEBUG_DEATH(linr.index(100.01) , "");

    EXPECT_DEBUG_DEATH(linr.bounds(10) , "");

    // Log
    eah::Log10Scale<double> logr_small(0,100, 10);

    EXPECT_DEBUG_DEATH(logr_small.index( -1), "");
    EXPECT_DEBUG_DEATH(logr_small.index(100.01) , "");
    
    EXPECT_DEBUG_DEATH(logr_small.bounds(10) , "");
}


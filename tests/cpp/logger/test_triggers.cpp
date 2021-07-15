#include <iostream>

#include "ioh.hpp"
#include <gtest/gtest.h> 


using namespace ioh;

TEST(trigger, always) {

    problem::MetaData pb(0,0,"fake",2);
    logger::Info e0;     e0.evaluations = 0;
    logger::Info e10;   e10.evaluations = 10;
    logger::Info e100; e100.evaluations = 100;

    trigger::Always always;
    EXPECT_TRUE(always(e0,pb));
    EXPECT_TRUE(always(e10,pb));
    EXPECT_TRUE(always(e100,pb));
}


TEST(trigger, on_improvement) {

    problem::MetaData pb(0,0,"fake",2); // Minimization
    logger::Info i;
    i.transformed_y = 9999;

    trigger::OnImprovement t;

    // First call.
    EXPECT_TRUE(t(i,pb)); // Better than infinity.

    i.transformed_y = 100;
    EXPECT_TRUE(t(i,pb)); // Improvement.
    EXPECT_FALSE(t(i,pb)); // Strict inequality.

    // Improvement.
    i.transformed_y = 10;
    EXPECT_TRUE(t(i,pb)); // Improvement.
    EXPECT_FALSE(t(i,pb)); // Strict inequality.

    // Unsuspected increasing.
    i.transformed_y = 99;
    EXPECT_FALSE(t(i,pb)); // Not improving.

    // Internal state stability.
    i.transformed_y = 11;
    EXPECT_FALSE(t(i,pb)); // Still worst than internal test.

    i.transformed_y = 9;
    EXPECT_TRUE(t(i,pb)); // Better than internal state.

    t.reset();
    i.transformed_y = 99;
    EXPECT_TRUE(t(i,pb)); // Better than infinity.
}

TEST(trigger, at) {
    problem::MetaData pb(0,0,"fake",2);
    logger::Info e0;     e0.evaluations = 0;
    logger::Info e10;   e10.evaluations = 10;
    logger::Info e11;   e11.evaluations = 11;
    logger::Info e99;   e99.evaluations = 99;
    logger::Info e100; e100.evaluations = 100;
    logger::Info e101; e101.evaluations = 101;

    // auto t = trigger::At( {999,1,100,10} ); // Classical stack instantiation.
    auto& t = trigger::at( {999,1,10,100} ); // On-the-fly heap instantiation.
    EXPECT_FALSE(t(e0  ,pb));
    EXPECT_TRUE (t(e10 ,pb));
    EXPECT_FALSE(t(e11 ,pb));
    EXPECT_FALSE(t(e99 ,pb));
    EXPECT_TRUE (t(e100,pb));
    EXPECT_FALSE(t(e101,pb));
    delete &t; // Having saved the reference, you can delete its address, as `triggers::at` allocated on the heap.
}

TEST(trigger, each) {
    problem::MetaData pb(0,0,"fake",2);
    logger::Info e0;     e0.evaluations = 0;
    logger::Info e10;   e10.evaluations = 10;
    logger::Info e11;   e11.evaluations = 11;
    logger::Info e19;   e19.evaluations = 19;
    logger::Info e20;   e20.evaluations = 20;
    logger::Info e21;   e21.evaluations = 21;
    logger::Info e100; e100.evaluations = 100;
    logger::Info e101; e101.evaluations = 101;

    auto t0 = trigger::Each(10,0);
    EXPECT_TRUE (t0(e0,pb));
    EXPECT_TRUE (t0(e10,pb));
    EXPECT_FALSE(t0(e11,pb));
    EXPECT_FALSE(t0(e19,pb));
    EXPECT_TRUE (t0(e20,pb));
    EXPECT_FALSE(t0(e21,pb));
    EXPECT_TRUE (t0(e100,pb));
    EXPECT_FALSE(t0(e101,pb));

    auto t1 = trigger::Each(10,1);
    EXPECT_FALSE(t1(e0,pb));
    EXPECT_FALSE(t1(e10,pb));
    EXPECT_TRUE (t1(e11,pb));
    EXPECT_FALSE(t1(e19,pb));
    EXPECT_FALSE(t1(e20,pb));
    EXPECT_TRUE (t1(e21,pb));
    EXPECT_FALSE(t1(e100,pb));
    EXPECT_TRUE (t1(e101,pb));
}


TEST(trigger, during) {
    problem::MetaData pb(0,0,"fake",2);
    logger::Info e0;     e0.evaluations = 0;
    logger::Info e10;   e10.evaluations = 10;
    logger::Info e18;   e18.evaluations = 18;
    logger::Info e19;   e19.evaluations = 19;
    logger::Info e100; e100.evaluations = 100;
    logger::Info e101; e101.evaluations = 101;
    logger::Info e102; e102.evaluations = 102;

    auto t = trigger::During({ {10,18}, {101,101} });
    EXPECT_FALSE(t(e0,pb));
    EXPECT_TRUE (t(e10,pb));
    EXPECT_TRUE (t(e18,pb));
    EXPECT_FALSE(t(e19,pb));
    EXPECT_FALSE(t(e100,pb));
    EXPECT_TRUE (t(e101,pb));
    EXPECT_FALSE(t(e102,pb));
}


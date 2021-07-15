#include <list>
#include <gtest/gtest.h>
#include "ioh.hpp"

TEST(store, data_consistency)
{
    using namespace ioh;

    auto sample_size = 10;
    auto nb_runs = 2;

    suite::BBOB suite({1, 2}, {1, 2}, {3, 10}); // problems, instances, dimensions
    // auto what = ioh::watch::TransformedY();
    // auto when = trigger::Always();
    // ioh::logger::Store logger({when},{what});
    trigger::Always always;
    watch::TransformedY transformed_y;
    logger::Store logger({always},{transformed_y});

    suite.attach_logger(logger);

    for (const auto &pb : suite) {
        for (auto r = 0; r < nb_runs; r++) {
            for (auto s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, 0));
            }
            pb->reset();
        }
    }

    EXPECT_EQ(logger.data().at(suite.name()).size(), 2); // 2 problems
    EXPECT_EQ(logger.data().at(suite.name()).at(/*pb*/1).size(), 2); // 2 dimensions
    EXPECT_EQ(logger.data().at(suite.name()).at(/*pb*/1).at(/*dim*/3).size(), 2); // 2 instances
    EXPECT_EQ(logger.data().at(suite.name()).at(/*pb*/1).at(/*dim*/10).at(/*ins*/2).size(), nb_runs);
    EXPECT_EQ(logger.data().at(suite.name()).at(/*pb*/1).at(/*dim*/10).at(/*ins*/2).at(/*run*/0).size(), sample_size);
    EXPECT_EQ(logger.data().at(suite.name()).at(/*pb*/1).at(/*dim*/10).at(/*ins*/2).at(/*run*/0).at(/*eval*/0).size(), 1); // 1 property watched
    EXPECT_GT(logger.data().at(suite.name()).at(/*pb*/1).at(/*dim*/10).at(/*ins*/2).at(/*run*/0).at(/*eval*/0).at(transformed_y.name()), 0);

}

TEST(store, properties)
{
    using namespace ioh;

    auto sample_size = 4;
    auto nb_runs = 2;

    suite::BBOB suite({1, 2}, {1, 2}, {3, 10}); // problems, instances, dimensions

    double my_attribute = 0;
    watch::Reference attr("Att_reference", my_attribute);
    watch::Pointer   attp("Att_pointer"  ,&my_attribute);

    double* nullp = nullptr;
    EXPECT_DEBUG_DEATH( watch::Pointer nope("Nope", nullp), "");

    double* p_transient_att = nullptr;
    watch::PointerReference attpr("Att_PtrRef", p_transient_att);
    
    trigger::Always always;
    watch::Evaluations evaluations;
    watch::RawYBest raw_y_best;
    watch::TransformedY transformed_y;
    watch::TransformedYBest transformed_y_best;
    logger::Store logger({always},{evaluations, raw_y_best, transformed_y, transformed_y_best, attr, attp, attpr});

    suite.attach_logger(logger);

    for (const auto &pb : suite) {
        for (auto r = 0; r < nb_runs; r++) {
            for (auto s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, 0));
                my_attribute++;
                if(s > sample_size/2) {
                    p_transient_att = & my_attribute;
                } else {
                    p_transient_att = nullptr;
                }
            }
            pb->reset();
        }
    }

    logger::Store::Cursor first_eval(suite.name(), /*pb*/1, /*dim*/3, /*ins*/1, /*run*/0, /*eval*/0);
    auto evals = logger.at(first_eval, evaluations);
    ASSERT_NE(evals, std::nullopt);
    EXPECT_GT(evals, 0);

    ASSERT_NE(logger.at(first_eval, attp ), std::nullopt);
    ASSERT_NE(logger.at(first_eval, attr ), std::nullopt);
    ASSERT_EQ(logger.at(first_eval, attpr), std::nullopt);
    
    ASSERT_EQ(logger.at(first_eval, attp ).value(), 0);
    ASSERT_EQ(logger.at(first_eval, attr ).value(), 0);

    logger::Store::Cursor last_eval(suite.name(), /*pb*/1, /*dim*/3, /*ins*/1, /*run*/0, /*eval*/sample_size-1);
    ASSERT_EQ(logger.at(last_eval, attp ).value(), 3);
    ASSERT_EQ(logger.at(last_eval, attr ).value(), 3);
    ASSERT_EQ(logger.at(last_eval, attpr), std::nullopt);

}


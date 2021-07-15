#include <gtest/gtest.h>
#include "ioh.hpp"

using namespace ioh;

TEST(eaf, logger)
{
    size_t sample_size = 100;
    size_t nb_runs = 2;

    suite::BBOB suite({1, 2}, {1, 2}, {10, 30});
    logger::EAF logger;

    IOH_DBG(debug,"Attach suite " << suite.name() << " to logger");
    suite.attach_logger(logger);

    for(const auto& pb : suite) {
        IOH_DBG(progress, "pb:" << pb->meta_data().problem_id << ", dim:" << pb->meta_data().n_variables << ", ins:" << pb->meta_data().instance);
        for(size_t r = 0; r < nb_runs; ++r) {
            IOH_DBG(progress, "> run:" << r);
            for(size_t s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, s));
            }
            pb->reset();
        }
    }

    auto& data = logger.data();
    EXPECT_EQ(data.size(), 1); // One suite
    EXPECT_EQ(data.at(suite.name()).size(),2); // 2 problems

    IOH_DBG(progress, "Result fronts:");
    size_t nb_fronts = 0;
    for(int pb : {1,2}) {
        EXPECT_EQ(data.at(suite.name()).at(pb).size(), 2); // 2 dimensions
        for(int dim : {10,30}) {
            EXPECT_EQ(data.at(suite.name()).at(pb).at(dim).size(), 2); // 2 instances
            for(int ins : {1,2}) {
                EXPECT_EQ(data.at(suite.name()).at(pb).at(dim).at(ins).size(), nb_runs); // 2 runs
                for(size_t run=0; run<nb_runs; ++run) {
                    logger::EAF::Cursor here(suite.name(), pb, dim, ins, run);
                    auto front = logger.data(here);
                    IOH_DBG(progress,"> Front size=" << front.size());
                    EXPECT_GT(front.size(), 0);
                    EXPECT_LE(front.size(), sample_size);
                    nb_fronts++;

                    // Check front dominance.
                    std::sort( std::begin(front), std::end(front), logger::eaf::ascending_time);
                    for(size_t i=1; i < front.size(); ++i) {
                        // Check non-dominance: the level should be a Pareto front.
                        // i.e. if sorted on ascending time, every consecutive quality should decrease. // FIXME maximization
                        EXPECT_GE(front[i-1].qual, front[i].qual);
                    }
    }}}}
    EXPECT_EQ(nb_fronts, 2*2*2*nb_runs);

}

TEST(eaf, all_levels)
{
    size_t sample_size = 100;
    size_t nb_runs = 10;

    suite::BBOB suite({1, 2}, {1, 2}, {10, 30});
    logger::EAF logger;

    suite.attach_logger(logger);

    for(const auto& pb : suite) {
        for(size_t r = 0; r < nb_runs; ++r) {
            for(size_t s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, s));
            }
            pb->reset();
        }
    }

    logger::eaf::Levels all_levels_of(common::OptimizationType::Minimization);
    auto levels = all_levels_of(logger);

    IOH_DBG(progress, levels.size() << " resulting attainment levels:");
    EXPECT_GT(levels.size(),0);
    std::set<size_t> evals;
    for(auto [level,front] : levels) {
        IOH_DBG(progress, "> Level " << level << " size=" << front.size());
        EXPECT_GT(front.size(), 0);
        EXPECT_LE(front.size(), sample_size);

        // Check level dominance.
        std::sort( std::begin(front), std::end(front), logger::eaf::ascending_time);
        ASSERT_GT(front.size(),0);
        evals.insert(front[0].time);
        for(size_t i=1; i < front.size(); ++i) {
            // Check non-dominance: the level should be a Pareto front.
            // i.e. if sorted on ascending time, every consecutive quality should decrease. // FIXME maximization
            EXPECT_GE(front[i-1].qual, front[i].qual);
            evals.insert(front[i].time);
        }
    } // level_front in levels
    EXPECT_EQ(levels.size(), 2*2*2*nb_runs);
    
    IOH_DBG(debug, "Write the levels in `eaf.csv`");
    CLUTCHCODE(debug,
        std::ofstream out("eaf.csv");
        const std::string sep = "\t";
        // for(const auto& level_front : levels) {
        for(auto [level,front] : levels) {
            std::sort(std::begin(front), std::end(front), logger::eaf::ascending_time);
            for(const auto& p : front) {
                out << level << sep << p.time << sep << p.qual << std::endl;
            }
        }
    );

}

TEST(eaf, some_levels)
{
    size_t sample_size = 10;
    size_t nb_runs = 10;

    suite::BBOB suite({1, 2}, {1, 2}, {10, 30});
    logger::EAF logger;

    suite.attach_logger(logger);

    for(const auto& pb : suite) {
        for(size_t r = 0; r < nb_runs; ++r) {
            for(size_t s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, s));
            }
            pb->reset();
        }
    }

    logger::eaf::Levels levels_at(common::OptimizationType::Minimization, {0, nb_runs/2, nb_runs-1});
    auto levels = levels_at(logger);
    EXPECT_EQ(levels.size(), 3);
}


TEST(eaf, levels_volume)
{
    size_t sample_size = 10;
    size_t nb_runs = 10;

    suite::BBOB suite({1, 2}, {1, 2}, {10, 30});
    logger::EAF logger;

    suite.attach_logger(logger);

    for(const auto& pb : suite) {
        for(size_t r = 0; r < nb_runs; ++r) {
            for(size_t s = 0; s < sample_size; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, s));
            }
            pb->reset();
        }
    }

    auto levels = logger::eaf::levels(common::OptimizationType::Minimization, logger);

    double volume = logger::eaf::stat::volume(common::OptimizationType::Minimization, levels);
    IOH_DBG(progress, "EAF volume: " << volume);
    EXPECT_GT(volume, 0);
    // EXPECT_LT(volume, nb_runs * sample_size * FIXME );
}


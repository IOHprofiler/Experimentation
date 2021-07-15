#include "ioh.hpp"
#include <gtest/gtest.h>

TEST(eah, stats)
{
    using namespace ioh::logger;

    size_t runs = 10;
    size_t sample_size = 100;
    size_t buckets = 10;

    size_t pb_start = 2;
    size_t pb_end = 10;
    ioh::suite::BBOB suite({ 1, 2 }, { 1, 2 }, { 2, 10 });
    EAH eah(0, 6e7, buckets, 0, sample_size, buckets);
    suite.attach_logger(eah);

    for (const auto& pb : suite) {
        for (size_t run = 0; run < runs; ++run) {
            // FIXME how to indicate different runs to the logger?
            // eah.update_run_info(pb->meta_data());
            for (auto s = 0; s < sample_size; ++s) {
                (*pb)(ioh::common::random::uniform(pb->meta_data().n_variables, 0));
            } // s
            pb->reset();
        } // run
    } // pb

    EXPECT_GT(eah::stat::sum(eah), 0);

    // Histogram
    eah::stat::Histogram histo;
    eah::stat::Histogram::Mat m = histo(eah);
    EXPECT_EQ(histo.nb_attainments(), runs * (pb_end - pb_start));
    
    // buckets * buckets matrix
    EXPECT_EQ(m.size(), buckets);
    for (const auto& row : m) {
        EXPECT_EQ(row.size(), buckets);
    }

    EXPECT_EQ(eah::stat::histogram(eah), m);
    
    // Distribution
    eah::stat::Distribution::Mat d = eah::stat::distribution(eah);
    
    // buckets * buckets matrix
    EXPECT_EQ(d.size(), buckets);
    for (const auto& row : d) {
        EXPECT_EQ(row.size(), buckets);
    }

    // Volume under curve
    EXPECT_GE(eah::stat::under_curve::volume(eah), 0);
    EXPECT_LE(eah::stat::under_curve::volume(eah), 1);
}

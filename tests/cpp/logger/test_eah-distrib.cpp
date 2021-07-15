#include "ioh.hpp"
#include <gtest/gtest.h>

template<class RR, class RV>
void do_test(std::string scale_errors, std::string scale_evals)
{
    using namespace ioh::logger;

    size_t runs = 13;
    size_t sample_size = 1000;
    size_t buckets = 100;

    size_t pb_start = 2;
    size_t pb_end = 10;
    ioh::suite::BBOB suite({ 1, 2 }, { 1, 2 }, { 2, 10 });

    RR r_error(0,6e7,buckets);
    RV r_evals(0,sample_size,buckets);
    
    EAH eah(r_error, r_evals);
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

    CLUTCHCODE(info,
        auto d = eah::stat::distribution(eah);
        std::clog << scale_errors << "-" << scale_evals << " joint cumulative attainment distribution for errors and evaluations:" << std::endl;
        std::clog << eah::colormap(d, {&r_error, &r_evals}, true) << std::endl;
        // std::clog << eah::colormap(d, {&r_error, &r_evals}) << std::endl;
        // std::clog << eah::colormap(d) << std::endl;
        // 
        auto h = eah::stat::histogram(eah);
        std::clog << scale_errors << "-" << scale_evals << " joint cumulative attainment histogram for errors and evaluations:" << std::endl;
        std::clog << eah::colormap(h, {&r_error, &r_evals}, true) << std::endl;
        // std::clog << eah::colormap(h, {&r_error, &r_evals}) << std::endl;
        // std::clog << eah::colormap(h) << std::endl;
    );
}

TEST(eah, plots)
{
    using namespace ioh::logger::eah;

    do_test<LinearScale<double>, LinearScale<size_t>>("linear","linear");
    do_test<  Log2Scale<double>, LinearScale<size_t>>("log2"  ,"linear");
    do_test<LinearScale<double>,   Log2Scale<size_t>>("linear","log2");
    do_test<  Log2Scale<double>,   Log2Scale<size_t>>("log2"  ,"log2");
    do_test< Log10Scale<double>,  Log10Scale<size_t>>("log10" ,"log10");
}

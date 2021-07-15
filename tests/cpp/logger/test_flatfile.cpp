#include <fstream>
#include <gtest/gtest.h>

#include "ioh.hpp"

using namespace ioh;

std::string get_file_as_string(const fs::path& path) {
    std::ifstream t(path);
    std::string str((std::istreambuf_iterator<char>(t)),
        std::istreambuf_iterator<char>());
    t.close();
    return str;
}

TEST(logger, flatfile)
{
    auto p0 = problem::bbob::Sphere(1, 2);
    auto p1 = problem::bbob::Sphere(1, 4);
    auto p2 = problem::bbob::AttractiveSector(1, 4);

    trigger::Always always;
    watch::TransformedY transformed_y;
    auto logger = logger::FlatFile( {always}, {transformed_y}, "IOH.dat", "." );

    const int runs = 3;
    const int samples = 3;

    for(auto pb : std::array<problem::BBOB*,3>({&p0,&p1,&p2})) {
        pb->attach_logger(logger);
        for(auto r = 0; r < runs; ++r) {
            for(auto s = 0; s < samples; ++s) {
                (*pb)(common::random::uniform(pb->meta_data().n_variables, s));
            }
            pb->reset();
        }
    }

    EXPECT_TRUE(fs::exists("./IOH.dat"));
    EXPECT_GT(get_file_as_string("./IOH.dat").size(),0);
}

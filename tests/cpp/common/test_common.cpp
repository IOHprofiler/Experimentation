#include <gtest/gtest.h>

#include "ioh.hpp"


TEST(common, test)
{
	using namespace ioh::common;

	EXPECT_TRUE(compare_objectives(4, 2, OptimizationType::Maximization));
	EXPECT_FALSE(compare_objectives(2, 2, OptimizationType::Maximization));

	EXPECT_TRUE(compare_objectives(2, 5, OptimizationType::Minimization));
	EXPECT_FALSE(compare_objectives(4, 2, OptimizationType::Minimization)); 
}

 
TEST(common, log)
{
    auto& ioh_dbg = clutchlog::logger();
    ioh_dbg.threshold(clutchlog::level::xdebug);

	testing::internal::CaptureStderr();
	ioh_dbg.format("{msg}");
	IOH_DBG(info,"Hello");
	auto output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(output, "Hello\x1B[0m"); // Hello + color reset ANSI code
}

TEST(common, typenames)
{
    EXPECT_EQ(ioh::common::class_name<ioh::problem::bbob::Sphere>(), "Sphere");
}



TEST(common, unique_folder) {
    using namespace ioh::common::file;
    const std::string f_name = "TEST_FOLDER";
    remove_all(fs::current_path() / f_name);

    const auto f = UniqueFolder(f_name);
    EXPECT_TRUE(fs::exists(f.path()));
    EXPECT_EQ(f.name(), f_name);

    const auto f2 = UniqueFolder(f_name);
    EXPECT_TRUE(fs::exists(f2.path()));
    EXPECT_EQ(f2.name(), "TEST_FOLDER-1");

    f.remove();
    EXPECT_FALSE(fs::exists(f.path()));

    f2.remove();
    EXPECT_FALSE(fs::exists(f2.path()));
}


std::string get_contents(const fs::path& file) {
    std::string line;
    std::string contents;
    std::ifstream reader(file);
    if (reader.is_open()) {
	while (getline(reader, line)) {
	    contents += line;
	}
	reader.close();
    }
    return contents;
}


TEST(common, buffered_file) {
    using namespace ioh::common::file;
    const std::string f_name = "TEST_FILE";
    auto f = BufferedFileStream(f_name);
    EXPECT_TRUE(fs::exists(f.path()));
    EXPECT_EQ(get_contents(f.path()), "");
    f.write("Hallo");
    EXPECT_EQ(f.buffer(), "Hallo");
    EXPECT_EQ(get_contents(f.path()), "");
    f.flush();
    EXPECT_EQ(get_contents(f.path()), "Hallo");
    EXPECT_EQ(f.buffer(), "");
    f.remove();
    EXPECT_FALSE(fs::exists(f.path()));
}

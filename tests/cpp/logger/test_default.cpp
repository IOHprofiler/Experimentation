#include <fstream>
#include <gtest/gtest.h>

#include "ioh.hpp"


// fs::path get_dat_path(const fs::path& root, const ioh::problem::Real& p) {
//     using namespace ioh::common;
//     return root / string_format("data_f%d_%s", p.meta_data().problem_id, p.meta_data().name.c_str()) /
//            string_format("IOHprofiler_f%d_DIM%d.dat", p.meta_data().problem_id, p.meta_data().n_variables);
// }

// std::string get_file_as_string(const fs::path& path) {
//     std::ifstream t(path);
//     std::string str((std::istreambuf_iterator<char>(t)),
//         std::istreambuf_iterator<char>());
//     t.close();
//     return str;
// }

TEST(logger, default) {
    // using namespace ioh::logger;
    // using namespace ioh::common;
    // using namespace ioh::problem::bbob;
    // file::UniqueFolder l_folder;
    // auto p = Sphere(1, 4);
    // auto p1 = Sphere(1, 2);
    // auto p2 = AttractiveSector(1, 4);
    
    // const auto info_file_name1 = string_format("IOHprofiler_f%d_%s.info",
    //                                            p.meta_data().problem_id, p.meta_data().name.c_str());
    // const auto info_file_name2 = string_format("IOHprofiler_f%d_%s.info",
    //                 p2.meta_data().problem_id, p2.meta_data().name.c_str());
    // {
    //     auto l = Default();   
    //     l_folder = l.experiment_folder();

    //     EXPECT_TRUE(fs::exists(l_folder.path()));

    //     for (auto *problem: std::array<ioh::problem::BBOB*, 3>({&p, &p1, &p2})) {
    //         const auto x = std::vector<double>(problem->meta_data().n_variables, 0);
    //         for (auto count = 0; 2 > count; ++count) {
    //             problem->attach_logger(l);
    //             (*problem)(x);
    //             problem->reset();
    //         }
    //     }
            
    //     EXPECT_TRUE(fs::exists(l_folder.path() / info_file_name1));
    //     EXPECT_TRUE(fs::exists(l_folder.path() / info_file_name2));
    // } // force destructor of logger to be called, flushes to files

    // const std::string test_info1 =
    //     R"(suite = "No suite", funcId = 1, funcName = "Sphere", DIM = 4, maximization = "F", algId = "algorithm_name", algInfo = "algorithm_info")"
    //     "\n%\ndata_f1_Sphere/IOHprofiler_f1_DIM4.dat, 1:1|5.63729, 1:1|5.63729\n"
    //     R"(suite = "No suite", funcId = 1, funcName = "Sphere", DIM = 2, maximization = "F", algId = "algorithm_name", algInfo = "algorithm_info")"
    //     "\n%\ndata_f1_Sphere/IOHprofiler_f1_DIM2.dat, 1:1|1.40209, 1:1|1.40209";

    // const std::string test_info2 =
    //     R"(suite = "No suite", funcId = 6, funcName = "AttractiveSector", DIM = 4, maximization = "F", algId = "algorithm_name", algInfo = "algorithm_info")"
    //     "\n%\ndata_f6_AttractiveSector/IOHprofiler_f6_DIM4.dat, 1:1|28268.4, 1:1|28268.4";

    // EXPECT_EQ(0, get_file_as_string(l_folder.path() / info_file_name1).compare(test_info1));
    // EXPECT_EQ(0, get_file_as_string(l_folder.path() / info_file_name2).compare(test_info2));
        
    // const std::string header = R"#("function evaluation" "current f(x)" "best-so-far f(x)" "current af(x)+b" "best af(x)+b")#";
    // std::array<std::pair<std::string, fs::path>, 3> cases;
    // cases[0] = { header + "\n1 1.402094 1.402094 80.882094 80.882094\n", get_dat_path(l_folder.path(),  p1) };
    // cases[1] = { header + "\n1 5.637287 5.637287 85.117287 85.117287\n", get_dat_path(l_folder.path(),  p) };
    // cases[2] = { header + "\n1 28268.397464 28268.397464 28304.297464 28304.297464\n", get_dat_path(l_folder.path(),  p2) };

    // for (const auto&[data, path]: cases)
    //     EXPECT_EQ(0, get_file_as_string(path).compare(data + data)) << "EXPECTED:\n" <<
    //         data + data << "\nGOT:\n" << get_file_as_string(path);
        
    // //
    // l_folder.remove();
    // EXPECT_TRUE(!fs::exists(l_folder.path()));
}

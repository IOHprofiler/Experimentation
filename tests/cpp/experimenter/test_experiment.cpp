#include <algorithm>
#include <iostream>
#include <gtest/gtest.h>

#include "ioh.hpp"

// #include "../utils.hpp" 


// void bbob_random_search(const std::shared_ptr<ioh::problem::Real>& p)
// {
// 	using namespace ioh::common;
// 	std::vector<double> x(p->meta_data().n_variables);
// 	auto count = 0;
// 	while (count++ < 10)
// 	{
//         (*p)(random::uniform(p->meta_data().n_variables, random::integer()));
// 	}
// }

// void pbo_random_search(const std::shared_ptr<ioh::problem::Integer>& p)
// {
// 	using namespace ioh::common;
// 	auto count = 0;
// 	while (count++ < 10)
// 	{
// 		const auto x = random::integers(p->meta_data().n_variables, 0, 5);
// 		(*p)(x);
// 	}
// }


// int count_newlines(const std::string& s)
// {
// 	return static_cast<int>(std::count_if(s.begin(), s.end(),
// 	                     [](const char& ch) { return ch == '\n'; }));
// }

TEST(experiment, config)
{
 //    auto config_file = find_test_file("conf.ini");
	// const ioh::experiment::Configuration conf(config_file);
	// EXPECT_EQ(conf.suite_name(), "PBO");
	// EXPECT_EQ(conf.problem_ids(), std::vector<int>({ 1, 2, 3, 4, 5 }));
	// EXPECT_EQ(conf.instances(), std::vector<int>({ 1 }));
	// EXPECT_EQ(conf.dimensions(), std::vector<int>({ 16 }));
	// EXPECT_EQ(conf.output_directory(), "./");
	// EXPECT_EQ(conf.result_folder(), "Experiment");
	// EXPECT_EQ(conf.algorithm_info(), "An_EA_algorithm");
	// EXPECT_EQ(conf.algorithm_name(), "(1+1)_EA");
	// EXPECT_EQ(conf.base_evaluation_triggers(), std::vector<int>({ 1 }));
	// EXPECT_EQ(conf.update_triggers(), true);
	// EXPECT_EQ(conf.complete_triggers(), false);
	// EXPECT_EQ(conf.number_target_triggers(), 0);
	// EXPECT_EQ(conf.number_interval_triggers(), 0);
}


TEST(experiment, pbo_from_config)
{
	// using namespace ioh;
 //    auto config_file = find_test_file("conf.ini");
	// experiment::Experimenter<problem::Integer> experiment(config_file, pbo_random_search);
	// EXPECT_EQ(experiment.independent_runs(), 1);
	// experiment.independent_runs(10);
	// EXPECT_EQ(experiment.independent_runs(), 10);
	// testing::internal::CaptureStdout();
	// experiment.run();
	// const auto output = testing::internal::GetCapturedStdout();
	// EXPECT_GE(count_newlines(output), 5);
	// // TODO: check that files are generated properly
	// experiment.logger()->flush();
	// try
	// {
	// 	fs::remove_all(dynamic_cast<logger::Default*>(&*experiment.logger())->experiment_folder().path()); // Cleanup	
	// }
	// catch (const std::exception& e)
	// {
	// 	IOH_DBG(warning,"Cannot remove directory: " 
	// 			<< static_cast<std::string>(e.what()));
	// }
}


TEST(experiment, bbob)
{
	// using namespace ioh;

	// std::vector<int> pbs = {1, 2};
	// std::vector<int> ins = {1, 2};
	// std::vector<int> dims = {2, 10};
	// const auto suite = std::make_shared<suite::BBOB>(pbs, ins, dims);
	// const auto logger = std::make_shared<logger::Default>(fs::current_path(),
	// 	std::string("logger-experimenter"), "random-search", "10iterations", common::OptimizationType::Minimization, true);
	// auto experiment = experiment::Experimenter<problem::Real>(
	// 	suite, logger, bbob_random_search, 10);

	// EXPECT_EQ(experiment.independent_runs(), 10);
	// testing::internal::CaptureStdout();
	// experiment.run();
	// const auto output = testing::internal::GetCapturedStdout();
	// EXPECT_GE(count_newlines(output), 8);
	// // TODO: check that files are generated properly
	// experiment.logger()->flush();
	// try
	// {
	// 	fs::remove_all(dynamic_cast<logger::Default*>(&*experiment.logger())->experiment_folder().path()); // Cleanup	
	// }
	// catch (const std::exception& e)
	// {
	// 	IOH_DBG(warning,"Cannot remove directory: "
	// 		<< static_cast<std::string>(e.what()));
	// }
}

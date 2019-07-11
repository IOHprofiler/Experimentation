## Using IOHexperimenter by C++

After compiling the tool by running `make` in the root directory, `/bin` and `/obj` are to be created in this folder. To use __IOHexperimenter__ to test your algorithms, you can work this `C` folder and replace the `Makefile` by the `Makefile-local`. Afterwards, you can edit your algorithm in the provided `cpp` files and compile them by `make` statement.

There are three ways to test algorithms:
* Test through problem class.
* Test through collections of problems (suite)
* Test through a designed experiment class and edit by configuration files

### Test through problem class
All problems of __IOHexperimenter__ are defined as specific `class` inheriting problem `IOHprofiler_problem` class, the implementation of problems are in the [problems folder](/src/Problems). To know the definition of problems, please visit the wiki page [https://iohprofiler.github.io/Benchmark/Problems/](https://iohprofiler.github.io/Benchmark/Problems/).

An example testing evolutionary algorithm with mutation operator on __OneMax__ is implemented in `IOHprofiler_run_problem.cpp`. 

For being tested, a `OneMax` class is declared and initialized as 1000 dimension. For the detail of instances, please visit [here](https://iohprofiler.github.io/Benchmark/Transformation/).
```cpp
OneMax om;
int dimension = 1000;
om.Initilize_problem(dimension);
```

During the optimization process, the algorithm can acquire the fitness value through <i>evaluate()</i> function. In the example below, <i>om.evaluate(x)</i> returns the fitness of `x`. You can also use the statement <i>om.evaluate(x,y)</i> to store the fitness of `x` in `y`. In addition, <i>om.IOHprofiler_hit_optimal()</i> is a detector to check if the optimum has been found.
```cpp
while(!om.IOHprofiler_hit_optimal()) {
  copyVector(x_star,x);
  if(mutation(x,mutation_rate)) {
    y = om.evaluate(x);
  }
  if(y[0] > best_value) {
    best_value = y[0];
    copyVector(x,x_star);
  }
}

If you want to generate result data for __IOHanalyzer__, a `IOHprofiler_csv_logger` should be added. The arguments of `IOHprofiler_csv_logger` are directory of result folder, name of result folder, name of the algorithm and infomation of the algorithm. In addition, you can set up mutilple triggers of recording evaluations. For the details of triggers, please visit the introduction of [`IOHprofiler_observer`](/src/Template/).
```cpp
std::vector<int> time_points{1,2,5};
std::shared_ptr<IOHprofiler_csv_logger> logger(new IOHprofiler_csv_logger("./","run_problem","EA","EA"));
logger->set_complete_flag(true);
logger->set_interval(0);
logger->set_time_points(time_points,10);
logger->activate_logger();
om.addCSVLogger(std::move(logger));
```

### Test through suite
Suites are collections of test problems. The idea is packing problems with similar/same properties toghther and making it easier to test algorithms on a class of problems. Currently a suite called __PBO__ consisting of 23 __pseudo Boolean problems__ are provied by __IOHexperimenter__. To create your own suites, please visit [here](/src/Suites).

An example testing evolutionary algorithm with mutation operator on __PBO__ suite is implemented in `IOHprofiler_run_suite.cpp`. __PBO__ suite includes pointers for 23 problems. To instantiate problems you want to test, the vectors of problem id, instances and dimensions need to be given.
```cpp
std::vector<int> problem_id = {1,2};
std::vector<int> instance_id ={1,2};
std::vector<int> dimension = {100,200,300};
PBO_suite pbo(problem_id,instance_id,dimension);
```

With the suite, you can test problems of the suite one by one, until all problems have been tested. In this example, the order of problem is as follow, and an `evlutionary_algorithm` is applied:

1. problem id 1, instance 1, dimension 100
2. problem id 1, instance 2, dimension 100
3. problem id 1, instance 1, dimension 200
4. problem id 1, instance 2, dimension 200
5. problem id 1, instance 1, dimension 300
6. problem id 1, instance 2, dimension 300
7. problem id 2, instance 1, dimension 100
8. problem id 2, instance 2, dimension 100
9. problem id 2, instance 1, dimension 200
10. problem id 2, instance 2, dimension 200
11. problem id 2, instance 1, dimension 300
12. problem id 2, instance 2, dimension 300

```cpp
while (problem = pbo.get_next_problem()) {
  evolutionary_algorithm(problem);
}
```

If you want to generate result data for __IOHanalyzer__, a `IOHprofiler_csv_logger` should be added. The arguments of `IOHprofiler_csv_logger` are directory of result folder, name of result folder, name of the algorithm and infomation of the algorithm. In addition, you can set up mutilple triggers of recording evaluations. For the details of triggers, please visit the introduction of [IOHprofiler_observer](/src/Template).
```cpp
std::vector<int> time_points{1,2,5};
std::shared_ptr<IOHprofiler_csv_logger> logger(new IOHprofiler_csv_logger("./","run_suite","EA","EA"));
logger->set_complete_flag(true);
logger->set_interval(2);
logger->set_time_points(time_points,3);
logger->activate_logger();
pbo.addCSVLogger(logger);
```

### Test through configuration file

By using the provided `IOHprofiler_experiment` class, you can use a configuration file to configure both the suite and the logger for csv files. For the setting of configuration, please visit the description in the file [configuration.ini](/src/Template/Experiments). 

With a `function` of optimizer algorithm and assiging the path of configuration file, <i>experimenter._run()</i> will execute all tasks of the experiment.

In addition, you can set the test times on each problem by <i>experimenter._set_independent_runs(2)</i>.

```cpp
std::string configName = "./configuration.ini";
IOHprofiler_experimenter<int> experimenter(configName,evolutionary_algorithm);
experimenter._set_independent_runs(2);
experimenter._run();
```
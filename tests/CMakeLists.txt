## CMake build file for the ioh library test suite.
##
## This will build a Google Test test runner for all contained tests.
##
add_subdirectory(${EXTERNAL_DIR}/googletest EXCLUDE_FROM_ALL build)
include(GoogleTest)
enable_testing()

function(register_test test_name test_sources)
    add_executable(${test_name} ${test_sources} cpp/utils.hpp cpp/entrypoint.cpp)
    target_include_directories(${test_name} PRIVATE cpp)
    target_link_libraries(${test_name} PRIVATE ${PROJECT_NAME} gtest)

    # When testing with ctest, only show messages up to the Warning.
    gtest_discover_tests(${test_name} EXTRA_ARGS "Warning")
endfunction()

# Add all tests in single executable
file(GLOB SOURCES cpp/*/*.cpp)
register_test(test_${PROJECT_NAME} "${SOURCES}")

# Add test for each file seperately
foreach(test_source ${SOURCES}) 
    get_filename_component(test_fname "${test_source}" NAME_WE)
    register_test(${test_fname} "${test_source}")
endforeach()

 

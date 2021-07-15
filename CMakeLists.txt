## Build the ioh library.
##
cmake_minimum_required(VERSION 3.12)

project(ioh   
    VERSION 0.0.0.1
    LANGUAGES CXX  
) 
set(CMAKE_CXX_STANDARD 17)
set(EXTERNAL_DIR "${PROJECT_SOURCE_DIR}/external")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
 
option(BUILD_PYTHON_PACKAGE "Whether to build Python package" OFF)
option(BUILD_TESTS "Whether to build test cases with gtest" ON)
option(BUILD_EXAMPLE "Whether to build Python package" ON)  
option(BUILD_DOCS "Build docs" OFF) 
option(ENABLE_PYTHON_PROBLEMS "Enable the ability to call external Python modules as if they were an IOH problem" OFF)

if(MSVC)
    set(ENABLE_PYTHON_PROBLEMS FALSE) # The call-problem-from-python interface is broken on windows
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /02") 
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd ")
endif()  
 
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU") 
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++17 -lstdc++fs")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        link_libraries(stdc++fs)
        add_compile_definitions(FSEXPERIMENTAL)
    endif()
endif() 
 
# Define project targets.
file(GLOB_RECURSE HEADERS include/ioh *.hpp) 
  
# Exlude python problems if not enabled
if(NOT ENABLE_PYTHON_PROBLEMS)
    list(FILTER HEADERS EXCLUDE REGEX "include/ioh/problem/python/*")
endif() 

 
add_library(ioh INTERFACE)
add_library(ioh::ioh ALIAS ioh)

#Include external formatting lib
add_subdirectory(${EXTERNAL_DIR}/fmt)
target_link_libraries(ioh INTERFACE fmt::fmt-header-only)

# Include external clutchlog lib
include_directories(${EXTERNAL_DIR}/clutchlog)

# Add Python Library
if(ENABLE_PYTHON_PROBLEMS)
    find_package(PythonLibs REQUIRED)
    include_directories(${PYTHON_INCLUDE_DIRS})
    target_link_libraries(ioh INTERFACE ${PYTHON_LIBRARIES})
    add_compile_definitions(PYTHON_DEFINED)
endif()


target_include_directories(ioh
INTERFACE
    # Interface paths are relative to the installation path, not the current directory.
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(ioh
INTERFACE
    cxx_std_17
)

# Generate package config files.
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/ioh-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ioh-config.cmake
INSTALL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/ioh
) 

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/ioh-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)


# Define installation rules.
include(GNUInstallDirs)

install(TARGETS ioh fmt-header-only
    EXPORT ioh-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ioh-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/ioh-config-version.cmake
DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/ioh
)

install(EXPORT ioh-targets
    FILE ioh-targets.cmake
    NAMESPACE ioh::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ioh
)
 

# Export targets. If an external project adds this project to its build tree
# via `add_subdirectory()`, then `find_package()` will work without
# requiring this project to be installed.
export(EXPORT ioh-targets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/ioh-targets.cmake
    NAMESPACE ioh::
)
export(PACKAGE ioh)


# Add subdirectories to build.
if(BUILD_TESTS) 
    add_subdirectory(tests)
endif()

if(BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if(BUILD_DOCS)
    add_subdirectory(doc)
endif()

if(BUILD_PYTHON_PACKAGE)
    add_subdirectory(ioh)
endif()




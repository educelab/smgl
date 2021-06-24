cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(smeagol VERSION 0.8.0)

# Setup project directories
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Modules
include(FetchContent)
include(CMakeDependentOption)
include(CheckIncludeFileCXX)

# Find deps
include(FindDependencies)

# Build the library
add_subdirectory(smgl)

# Build the docs
find_package(Doxygen OPTIONAL_COMPONENTS dot)
CMAKE_DEPENDENT_OPTION(SMGL_BUILD_DOCS "Build Doxygen documentation" on "DOXYGEN_FOUND" off)
if(SMGL_BUILD_DOCS)
    add_subdirectory(docs)
endif()

# Tests #
option(SMGL_BUILD_TESTS "Compile smeagol unit tests" off)
if(SMGL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install the library
include(InstallProject)
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(smeagol VERSION 1.0.0)

# Setup project directories
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Public headers
set(public_hdrs
    include/smeagol/Ports.hpp
)

# smeagol lib #
add_library(smeagol INTERFACE)
add_library(smeagol::smeagol ALIAS smeagol)
target_include_directories(smeagol
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(smeagol INTERFACE cxx_std_14)

# Install Library ##
set_target_properties(smeagol
    PROPERTIES
        PUBLIC_HEADER ${public_hdrs}
)
install(
    TARGETS smeagol
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    INCLUDES DESTINATION
    PUBLIC_HEADER DESTINATION "include/smeagol"
)

# Tests #
option(SMEAGOL_BUILD_TESTS "Compile smeagol tests" on)
if(SMEAGOL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
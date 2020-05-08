cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(smeagol VERSION 1.0.0)

# Public headers
set(public_hdrs
    include/smeagol/Ports.hpp
)

# smeagol lib
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

### Test application
add_executable(smeagol-test src/ports-test.cpp)
target_link_libraries(smeagol-test smeagol::smeagol)
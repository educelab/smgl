## Google Test ##
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
        EXCLUDE_FROM_ALL
)
set(INSTALL_GTEST OFF CACHE BOOL OFF FORCE)
FetchContent_MakeAvailable(googletest)

## Test Library ##
add_library(testlib INTERFACE)
add_library(smgl::testlib ALIAS testlib)
target_include_directories(testlib
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(testlib INTERFACE smgl::smgl)

## Build the tests ##
set(tests
    src/TestPorts.cpp
    src/TestNode.cpp
    src/TestGraph.cpp
    src/TestSingleton.cpp
    src/TestUuid.cpp
    src/TestGraphviz.cpp
    src/TestLogging.cpp
)

foreach(src ${tests})
    get_filename_component(filename ${src} NAME_WE)
    set(testname smgl_${filename})
    add_executable(${testname} ${src})
    target_link_libraries(${testname}
        smgl::smgl
        smgl::testlib
        gtest_main
        gmock_main
    )
    add_test(
        NAME ${testname}
        WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
        COMMAND ${testname}
    )
endforeach()
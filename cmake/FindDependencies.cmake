## CXX ABI ##
check_include_file_cxx(cxxabi.h HAVE_CXXABI_H)
if(NOT HAVE_CXXABI_H)
    message(FATAL_ERROR "Required include not found: cxxabi.h")
endif()

## Modern JSON ##
option(SMGL_BUILD_JSON "Build in-source JSON library" ON)
if(SMGL_BUILD_JSON)
    FetchContent_Declare(
        json
        URL https://github.com/nlohmann/json/archive/v3.11.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP ON
    )

    FetchContent_GetProperties(json)
    if(NOT json_POPULATED)
        set(JSON_BuildTests OFF CACHE INTERNAL "")
        set(JSON_Install ON CACHE INTERNAL "")
        FetchContent_Populate(json)
        add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
else()
    find_package(nlohmann_json 3.9.1 REQUIRED)
endif()

## Filesystem ##
find_package(Filesystem)
option(SMGL_USE_BOOSTFS "Use Boost as the filesystem library" "NOT Filesystem_FOUND")
if(SMGL_USE_BOOSTFS)
    add_compile_definitions(SMGL_USE_BOOSTFS)
    find_package(Boost 1.58 REQUIRED COMPONENTS system filesystem)
    set(SMGL_FS_LIB Boost::filesystem)
else()
    set(SMGL_FS_LIB std::filesystem)
endif()
message(STATUS "Using filesystem library: ${SMGL_FS_LIB}")
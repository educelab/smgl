# Public headers
set(public_hdrs
    include/smgl/smgl.hpp
    include/smgl/Factory.hpp
    include/smgl/FactoryImpl.hpp
    include/smgl/filesystem.hpp
    include/smgl/Graph.hpp
    include/smgl/GraphImpl.hpp
    include/smgl/Graphviz.hpp
    include/smgl/GraphvizImpl.hpp
    include/smgl/Metadata.hpp
    include/smgl/Node.hpp
    include/smgl/NodeImpl.hpp
    include/smgl/Ports.hpp
    include/smgl/PortsImpl.hpp
    include/smgl/Singleton.hpp
    include/smgl/SingletonImpl.hpp
    include/smgl/TypeTraits.hpp
    include/smgl/Utilities.hpp
    include/smgl/UtilitiesImpl.hpp
    include/smgl/Uuid.hpp
)
# Source files
set(srcs
    src/Graph.cpp
    src/Graphviz.cpp
    src/Metadata.cpp
    src/Node.cpp
    src/Ports.cpp
    src/Utilities.cpp
    src/Uuid.cpp
)

add_library(smgl ${srcs})
add_library(smgl::smgl ALIAS smgl)
target_include_directories(smgl
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(smgl PUBLIC cxx_std_14)
target_link_libraries(smgl
    PUBLIC
        ${SMGL_FS_LIB}
        nlohmann_json::nlohmann_json
)
if(SMGL_USE_BOOSTFS)
    target_compile_definitions(smgl PUBLIC SMGL_USE_BOOSTFS)
endif()

# Install Library ##
set_target_properties(smgl
    PROPERTIES
        PUBLIC_HEADER "${public_hdrs}"
)
install(
    TARGETS smgl
    EXPORT smeagolTargets
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    INCLUDES DESTINATION "include/smgl"
    PUBLIC_HEADER DESTINATION "include/smgl"
)
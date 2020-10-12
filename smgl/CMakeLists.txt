# Public headers
set(public_hdrs
    include/smgl/Factory.hpp
    include/smgl/FactoryImpl.hpp
    include/smgl/filesystem.hpp
    include/smgl/Graph.hpp
    include/smgl/GraphImpl.hpp
    include/smgl/Metadata.hpp
    include/smgl/MetadataImpl.hpp
    include/smgl/Node.hpp
    include/smgl/NodeImpl.hpp
    include/smgl/Ports.hpp
    include/smgl/PortsImpl.hpp
    include/smgl/Singleton.hpp
    include/smgl/SingletonImpl.hpp
    include/smgl/Utilities.hpp
    include/smgl/UtilitiesImpl.hpp
    include/smgl/Uuid.hpp
    include/smgl/UuidImpl.hpp
)
add_library(smgl INTERFACE)
add_library(smgl::smgl ALIAS smgl)
target_include_directories(smgl
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(smgl INTERFACE cxx_std_14)
target_link_libraries(smgl
    INTERFACE
        ${SMGL_FS_LIB}
        nlohmann_json::nlohmann_json
)
if(SMGL_USE_BOOSTFS)
    target_compile_definitions(smgl INTERFACE SMGL_USE_BOOSTFS)
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
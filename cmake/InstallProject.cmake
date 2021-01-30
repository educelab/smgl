include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    smeagolConfigVersion.cmake
    COMPATIBILITY AnyNewerVersion
)
configure_file(cmake/smeagolConfig.cmake.in smeagolConfig.cmake @ONLY)

install(
    EXPORT smeagolTargets
    FILE smeagolTargets.cmake
    NAMESPACE smgl::
    DESTINATION lib/cmake/smeagol
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/smeagolConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/smeagolConfigVersion.cmake"
    DESTINATION lib/cmake/smeagol
)

install(
    FILES
        "${CMAKE_MODULE_PATH}/FindFilesystem.cmake"
    DESTINATION lib/cmake/smeagol/Modules
)
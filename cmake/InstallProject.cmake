include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    smglConfigVersion.cmake
    COMPATIBILITY AnyNewerVersion
)
configure_file(cmake/smglConfig.cmake.in smglConfig.cmake @ONLY)

install(
    EXPORT smglTargets
    FILE smglTargets.cmake
    NAMESPACE smgl::
    DESTINATION lib/cmake/smgl
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/smglConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/smglConfigVersion.cmake"
    DESTINATION lib/cmake/smgl
)

install(
    FILES
        "${CMAKE_MODULE_PATH}/FindFilesystem.cmake"
    DESTINATION lib/cmake/smgl/Modules
)
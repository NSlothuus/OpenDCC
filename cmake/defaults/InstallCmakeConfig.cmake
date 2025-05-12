include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/opendccConfigVersion.cmake"
    VERSION "${OPENDCC_VERSION_STRING}"
    COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/opendccConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/opendccConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/opendcc
    NO_CHECK_REQUIRED_COMPONENTS_MACRO)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/opendccConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/opendccConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/opendcc")

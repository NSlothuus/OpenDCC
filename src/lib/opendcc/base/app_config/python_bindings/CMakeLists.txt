set(TARGET_NAME py_app_config)
add_library(${TARGET_NAME} SHARED wrap_config.cpp)
target_include_directories(${TARGET_NAME} PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} app_config pybind11::pybind11 ${PYTHON_LIBRARY})
make_folder(base/app_config ${TARGET_NAME})

if(MSVC)
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES PREFIX ""
                   OUTPUT_NAME app_config
                   SUFFIX ".pyd")
else()
    set_target_properties(
        ${TARGET_NAME}
        PROPERTIES PREFIX ""
                   OUTPUT_NAME app_config
                   SUFFIX ".so")
endif()

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:${TARGET_NAME}>
        DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc
        OPTIONAL)
endif()
install(
    TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc
    LIBRARY DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc)

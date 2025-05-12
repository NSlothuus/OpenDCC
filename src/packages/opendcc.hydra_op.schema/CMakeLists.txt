set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.schema/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op/schema)

opendcc_make_usd_schema(
    opendcc.hydra_op.schema
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op.schema/pxr_plugins
    SCHEMA_FILE
    ${_src_dir}/schema.usda
    CPPFILES
    ${_src_dir}/moduleDeps.cpp
    SCHEMA_DEPENDENCIES
    usdUIExt
    CUSTOM_RUNTIME_DIR
    packages/opendcc.hydra_op.schema/lib
    PYMODULE_CPPFILES
    ${_src_dir}/module.cpp
    PYTHON_FILES
    ${_src_dir}/__init__.py
    HEADER_INSTALL_DIR
    "include/opendcc/hydra_op/schema"
    SRC_GENERATION_DIR
    opendcc/hydra_op/schema
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.schema/python")

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pxr_plugins/plugInfo.json DESTINATION ${PLUGIN_DIR}/pxr_plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

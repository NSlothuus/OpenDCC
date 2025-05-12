set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.anim_engine.schema/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/anim_engine/schema)

opendcc_make_usd_schema(
    opendcc.anim_engine.schema
    AFTER_GEN_SCRIPT
    ${_src_dir}/fix_generated.py
    ${USD_VERSION}
    VS_FOLDER
    packages/anim_engine
    PACKAGE_DIR
    opendcc.anim_engine.schema/pxr_plugins
    SCHEMA_FILE
    ${_src_dir}/schema.usda
    PYTHON_FILES
    ${_src_dir}/__init__.py
    HEADER_INSTALL_DIR
    "include/opendcc/anim_engine/schema"
    SRC_GENERATION_DIR
    opendcc/anim_engine/schema
    CUSTOM_RUNTIME_DIR
    packages/opendcc.anim_engine.schema/lib
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.anim_engine.schema/python")

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pxr_plugins/plugInfo.json DESTINATION ${PLUGIN_DIR}/pxr_plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

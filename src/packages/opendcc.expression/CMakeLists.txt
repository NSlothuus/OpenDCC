set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.expression/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)

set(_src_dir lib/opendcc/expression/engine)

if(WIN32)
    set(_runtime_dir bin)
else()
    set(_runtime_dir lib)
endif()

opendcc_make_usd_schema(
    expression_engine_usd_schema
    AFTER_GEN_SCRIPT
    usd_schema/fix_generated.py
    ${USD_VERSION}
    VS_FOLDER
    expression
    PACKAGE_DIR
    opendcc.expression/pxr_plugins
    SCHEMA_FILE
    usd_schema/schema.usda
    PYTHON_FILES
    usd_schema/__init__.py
    HEADER_INSTALL_DIR
    "include/opendcc/expression/usd_schema"
    SRC_GENERATION_DIR
    "opendcc/expression/usd_schema"
    CUSTOM_RUNTIME_DIR
    ${_runtime_dir}
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.expression/python/opendcc/expression")

opendcc_make_library(
    expression_engine_extension
    QT_AUTOMOC
    TYPE
    SHARED
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/engine.h
    ${_src_dir}/iexpression.h
    ${_src_dir}/session.h
    ${_src_dir}/entry_point.h
    ${_src_dir}/expression_factory.h
    ${_src_dir}/iexpression.h
    CPPFILES
    ${_src_dir}/engine.cpp
    ${_src_dir}/session.cpp
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/expression_factory.cpp
    ${_src_dir}/expand_vars_expression.cpp
    PRIVATE_DEFINITIONS
    EXPRESSION_ENGINE_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc_lib
    expression_engine_usd_schema
    VS_FOLDER
    packages/expression
    PYMODULE_NAME
    expression_engine
    PYMODULE_SOURCES
    ${_src_dir}/wrap_engine.cpp
    PYMODULE_INSTALL_PATH
    packages/opendcc.expression/python/opendcc/expression)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")
if(PYTHON_VERSION_MAJOR VERSION_LESS 3)
    install(
        CODE "file(WRITE \"${PLUGIN_DIR}/python/opendcc/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
endif()

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pxr_plugins/plugInfo.json DESTINATION ${PLUGIN_DIR}/pxr_plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

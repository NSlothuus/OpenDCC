set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.bullet_physics/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/bullet_physics)

opendcc_make_library(
    opendcc.usd_editor.bullet_physics
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/debug_drawer.h
    ${_src_dir}/engine.h
    ${_src_dir}/entry_point.h
    ${_src_dir}/session.h
    ${_src_dir}/utils.h
    CPPFILES
    ${_src_dir}/debug_drawer.cpp
    ${_src_dir}/engine.cpp
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/session.cpp
    ${_src_dir}/utils.cpp
    PRIVATE_DEFINITIONS
    BULLET_PHYSICS_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc_lib
    Bullet3::bullet
    VS_FOLDER
    packages/usd_editor
    PACKAGE_DIR
    opendcc.usd_editor.bullet_physics/lib
    PYMODULE_NAME
    bullet_physics
    PYMODULE_SOURCES
    ${_src_dir}/wrap/bullet_physics_py.cpp
    PYMODULE_INSTALL_PATH
    packages/opendcc.usd_editor.bullet_physics/python/opendcc/usd_editor/)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")
if(PYTHON_VERSION_MAJOR VERSION_LESS 3)
    install(
        CODE "file(WRITE \"${PLUGIN_DIR}/python/opendcc/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
    install(
        CODE "file(WRITE \"${PLUGIN_DIR}/python/opendcc/usd_editor/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
endif()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

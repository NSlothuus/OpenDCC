set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.anim_engine/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/anim_engine)

opendcc_make_library(
    opendcc.anim_engine
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/anim_engine_entry_point.h
    CPPFILES
    ${_src_dir}/anim_engine_entry_point.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    opendcc.anim_engine.ui.graph_editor
    VS_FOLDER
    packages/anim_engine
    PACKAGE_DIR
    opendcc.anim_engine/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

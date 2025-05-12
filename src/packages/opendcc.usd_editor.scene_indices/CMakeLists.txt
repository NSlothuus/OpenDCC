set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.scene_indices/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/scene_indices)

opendcc_make_library(
    opendcc.usd_editor.scene_indices
    TYPE
    SHARED
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/utils.h
    ${_src_dir}/prune_scene_index.h
    ${_src_dir}/isolate_scene_index.h
    CPPFILES
    ${_src_dir}/prune_scene_index.cpp
    ${_src_dir}/isolate_scene_index.cpp
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_DEFINITIONS
    OPENDCC_USD_EDITOR_SCENE_INDICES_EXPORT
    LIBRARY_DEPENDENCIES
    logging
    usdImaging
    hd
    VS_FOLDER
    usd/dcc)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.translator/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op/translator)

opendcc_make_library(
    opendcc.hydra_op.translator
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/registry.h
    ${_src_dir}/network.h
    ${_src_dir}/terminal_scene_index.h
    ${_src_dir}/node_translator.h
    ${_src_dir}/network_registry.h
    ${_src_dir}/utils.h
    PRIVATE_HEADERS
    ${_src_dir}/usd_in.h
    ${_src_dir}/set_attr.h
    ${_src_dir}/entry_point.h
    ${_src_dir}/merge.h
    ${_src_dir}/network_modifier.h
    ${_src_dir}/translate.h
    ${_src_dir}/isolate.h
    ${_src_dir}/prune.h
    ${_src_dir}/material_assign.h
    CPPFILES
    ${_src_dir}/registry.cpp
    ${_src_dir}/usd_in.cpp
    ${_src_dir}/merge.cpp
    ${_src_dir}/set_attr.cpp
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/network_modifier.cpp
    ${_src_dir}/network.cpp
    ${_src_dir}/network_registry.cpp
    ${_src_dir}/terminal_scene_index.cpp
    ${_src_dir}/translate.cpp
    ${_src_dir}/isolate.cpp
    ${_src_dir}/prune.cpp
    ${_src_dir}/material_assign.cpp
    PRIVATE_DEFINITIONS
    OPENDCC_HYDRA_OP_TRANSLATOR_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc_lib
    opendcc.hydra_op.schema
    opendcc.usd_editor.scene_indices
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op.translator/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

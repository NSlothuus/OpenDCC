set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.usd_node_editor/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/usd_node_editor)

opendcc_make_library(
    opendcc.usd_editor.usd_node_editor
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/backdrop_node.h
    ${_src_dir}/navigation_bar.h
    ${_src_dir}/item_registry.h
    ${_src_dir}/graph_model.h
    ${_src_dir}/node.h
    ${_src_dir}/node.cpp
    ${_src_dir}/oiio_thumbnail_cache.h
    ${_src_dir}/move_items_command.cpp
    ${_src_dir}/node_provider.h
    ${_src_dir}/utils.cpp
    ${_src_dir}/utils.h
    PRIVATE_HEADERS
    ${_src_dir}/node_disconnect_machine.h
    ${_src_dir}/disconnect_after_shake_command.h
    ${_src_dir}/move_items_command.h
    ${_src_dir}/group_prim_to_nodegraph.h
    ${_src_dir}/layout_command.h
    CPPFILES
    ${_src_dir}/backdrop_node.cpp
    ${_src_dir}/navigation_bar.cpp
    ${_src_dir}/item_registry.cpp
    ${_src_dir}/graph_model.cpp
    ${_src_dir}/node.cpp
    ${_src_dir}/node_disconnect_machine.cpp
    ${_src_dir}/disconnect_after_shake_command.cpp
    ${_src_dir}/oiio_thumbnail_cache.cpp
    ${_src_dir}/move_items_command.cpp
    ${_src_dir}/group_prim_to_nodegraph.cpp
    ${_src_dir}/node_provider.cpp
    ${_src_dir}/utils.cpp
    ${_src_dir}/layout_command.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    usdUI
    opendcc.ui.node_editor
    usd_fallback_proxy
    opendcc.usd_editor.common_cmds
    Graphviz
    usdUIExt
    PRIVATE_DEFINITIONS
    OPENDCC_USD_NODE_EDITOR_EXPORT
    VS_FOLDER
    packages/usd_editor
    PACKAGE_DIR
    opendcc.usd_editor.usd_node_editor/lib)

opendcc_make_shiboken_bindings(
    opendcc.usd_editor.usd_node_editor_py
    ${_verbose_shiboken_output}
    TARGET
    opendcc.usd_editor.usd_node_editor
    SOURCE_DIR
    ${_src_dir}/python
    GEN_CONFIG_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.usd_editor.common_cmds/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.ui.node_editor/lib
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    VS_FOLDER
    packages/usd_editor
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.usd_node_editor/python/opendcc/usd_editor/usd_node_editor")

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}/"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

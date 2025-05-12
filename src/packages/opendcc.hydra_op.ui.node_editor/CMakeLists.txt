set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.ui.node_editor/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op/ui/node_editor)

opendcc_make_library(
    opendcc.hydra_op.ui.node_editor
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/hydra_op_item_registry.h
    ${_src_dir}/hydra_op_graph_model.h
    ${_src_dir}/hydra_op_node_item.h
    ${_src_dir}/hydra_op_input_item.h
    ${_src_dir}/hydra_op_utils.h
    CPPFILES
    ${_src_dir}/hydra_op_item_registry.cpp
    ${_src_dir}/hydra_op_graph_model.cpp
    ${_src_dir}/hydra_op_node_item.cpp
    ${_src_dir}/hydra_op_input_item.cpp
    ${_src_dir}/hydra_op_utils.cpp
    LIBRARY_DEPENDENCIES
    opendcc.usd_editor.usd_node_editor
    expression_engine_extension
    opendcc.hydra_op
    PRIVATE_DEFINITIONS
    OPENDCC_HYDRA_OP_NODE_EDITOR_EXPORT
    RESOURCES
    ${_src_dir}/plugInfo.json
    RESOURCES_DIR
    packages/opendcc.hydra_op.ui.node_editor/pxr_plugins/opendcc.hydra_op.ui.node_editor/resources
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op.ui.node_editor/lib)

opendcc_make_shiboken_bindings(
    opendcc.hydra_op.ui.node_editor_py
    ${_verbose_shiboken_output}
    TARGET
    opendcc.hydra_op.ui.node_editor
    SOURCE_DIR
    ${_src_dir}/python
    GEN_CONFIG_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.usd_editor.common_cmds/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.hydra_op.ui.node_editor/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.ui.node_editor/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.usd_editor.usd_node_editor/lib
    VS_FOLDER
    packages/hydra_op
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.ui.node_editor/python/opendcc/hydra_op/ui/node_editor")

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pxr_plugins/plugInfo.json DESTINATION ${PLUGIN_DIR}/pxr_plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

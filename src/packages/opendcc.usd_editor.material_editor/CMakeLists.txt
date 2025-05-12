set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.material_editor/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/material_editor)

opendcc_make_library(
    opendcc.usd_editor.material_editor
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/material_item_registry.h
    ${_src_dir}/material_item_registry.cpp
    ${_src_dir}/model.h
    ${_src_dir}/model.cpp
    ${_src_dir}/shader_node.h
    ${_src_dir}/shader_node.cpp
    ${_src_dir}/material_output_item.h
    ${_src_dir}/material_output_item.cpp
    ${_src_dir}/nodegraph_item.h
    ${_src_dir}/nodegraph_item.cpp
    ${_src_dir}/utils.h
    ${_src_dir}/utils.cpp
    CPPFILES
    ${_src_dir}/material_item_registry.cpp
    ${_src_dir}/model.cpp
    ${_src_dir}/shader_node.cpp
    ${_src_dir}/material_output_item.cpp
    ${_src_dir}/nodegraph_item.cpp
    ${_src_dir}/utils.cpp
    LIBRARY_DEPENDENCIES
    opendcc.usd_editor.usd_node_editor
    PRIVATE_DEFINITIONS
    OPENDCC_MATERIAL_EDITOR_EXPORT
    VS_FOLDER
    packages/usd_editor
    PACKAGE_DIR
    opendcc.usd_editor.material_editor/lib)

opendcc_make_shiboken_bindings(
    opendcc.usd_editor.material_editor_py
    ${_verbose_shiboken_output}
    TARGET
    opendcc.usd_editor.material_editor
    SOURCE_DIR
    ${_src_dir}/python
    GEN_CONFIG_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.usd_editor.common_cmds/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.ui.node_editor/lib
    ${PROJECT_SOURCE_DIR}/src/packages/opendcc.usd_editor.usd_node_editor/lib
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    VS_FOLDER
    packages/usd_editor
    PYTHON_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.material_editor/python/opendcc/usd_editor/material_editor")

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}/"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.sculpt_tool/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/sculpt_tool)

opendcc_make_library(
    opendcc.usd_editor.sculpt_tool
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/sculpt_tool_context.h
    ${_src_dir}/sculpt_tool_settings.h
    ${_src_dir}/sculpt_tool_entry_point.h
    ${_src_dir}/mesh_manipulation_data.h
    ${_src_dir}/sculpt_functions.h
    ${_src_dir}/sculpt_strategies.h
    ${_src_dir}/sculpt_properties.h
    ${_src_dir}/utils.h
    CPPFILES
    ${_src_dir}/sculpt_tool_context.cpp
    ${_src_dir}/sculpt_tool_settings.cpp
    ${_src_dir}/sculpt_tool_entry_point.cpp
    ${_src_dir}/mesh_manipulation_data.cpp
    ${_src_dir}/sculpt_functions.cpp
    ${_src_dir}/sculpt_strategies.cpp
    ${_src_dir}/sculpt_properties.cpp
    ${_src_dir}/utils.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    opendcc.usd_editor.common_tools
    VS_FOLDER
    usd/dcc
    PACKAGE_DIR
    opendcc.usd_editor.sculpt_tool/lib)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

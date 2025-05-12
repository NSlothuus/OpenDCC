set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.bezier_tool/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/bezier_tool)

opendcc_make_library(
    opendcc.usd_editor.bezier_tool
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/bezier_tool_settings_widget.h
    ${_src_dir}/add_point_to_curve_command.h
    ${_src_dir}/change_curve_point_command.h
    ${_src_dir}/remove_curve_point_command.h
    ${_src_dir}/bezier_tool_strategy.h
    ${_src_dir}/close_curve_command.h
    ${_src_dir}/bezier_tool_context.h
    ${_src_dir}/bezier_curve.h
    ${_src_dir}/event_filter.h
    ${_src_dir}/entry_point.h
    ${_src_dir}/utils.h
    CPPFILES
    ${_src_dir}/bezier_tool_settings_widget.cpp
    ${_src_dir}/add_point_to_curve_command.cpp
    ${_src_dir}/change_curve_point_command.cpp
    ${_src_dir}/remove_curve_point_command.cpp
    ${_src_dir}/bezier_tool_strategy.cpp
    ${_src_dir}/close_curve_command.cpp
    ${_src_dir}/bezier_tool_context.cpp
    ${_src_dir}/bezier_curve.cpp
    ${_src_dir}/event_filter.cpp
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/utils.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    opendcc.usd_editor.common_tools
    VS_FOLDER
    usd/dcc
    PACKAGE_DIR
    opendcc.usd_editor.bezier_tool/lib)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

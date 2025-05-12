set(PKG_NAME opendcc.usd_editor.common_tools)

set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/${PKG_NAME}/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/common_tools)

opendcc_make_library(
    ${PKG_NAME}
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/entry_point.h
    ${_src_dir}/viewport_change_pivot_command.h
    ${_src_dir}/viewport_move_tool_command.h
    ${_src_dir}/viewport_move_tool_context.h
    ${_src_dir}/viewport_move_tool_settings_widget.h
    ${_src_dir}/viewport_pivot_editor.h
    ${_src_dir}/viewport_render_region_extension.h
    ${_src_dir}/viewport_render_region_tool_context.h
    ${_src_dir}/viewport_rotate_tool_command.h
    ${_src_dir}/viewport_rotate_tool_context.h
    ${_src_dir}/viewport_rotate_tool_settings_widget.h
    ${_src_dir}/viewport_scale_tool_command.h
    ${_src_dir}/viewport_scale_tool_context.h
    ${_src_dir}/viewport_scale_tool_settings_widget.h
    ${_src_dir}/viewport_select_tool_context.h
    ${_src_dir}/viewport_select_tool_settings_widget.h
    ${_src_dir}/viewport_usd_snap_strategy.h
    CPPFILES
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/viewport_change_pivot_command.cpp
    ${_src_dir}/viewport_move_tool_command.cpp
    ${_src_dir}/viewport_move_tool_context.cpp
    ${_src_dir}/viewport_move_tool_settings_widget.cpp
    ${_src_dir}/viewport_pivot_editor.cpp
    ${_src_dir}/viewport_render_region_extension.cpp
    ${_src_dir}/viewport_render_region_tool_context.cpp
    ${_src_dir}/viewport_rotate_tool_command.cpp
    ${_src_dir}/viewport_rotate_tool_context.cpp
    ${_src_dir}/viewport_rotate_tool_settings_widget.cpp
    ${_src_dir}/viewport_scale_tool_command.cpp
    ${_src_dir}/viewport_scale_tool_context.cpp
    ${_src_dir}/viewport_scale_tool_settings_widget.cpp
    ${_src_dir}/viewport_select_tool_context.cpp
    ${_src_dir}/viewport_select_tool_settings_widget.cpp
    ${_src_dir}/viewport_usd_snap_strategy.cpp
    PRIVATE_DEFINITIONS
    OPENDCC_USD_EDITOR_COMMON_TOOLS_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc_lib
    VS_FOLDER
    packages/usd_editor
    PACKAGE_DIR
    ${PKG_NAME}/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

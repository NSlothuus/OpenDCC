set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.texture_paint/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/texture_paint)

opendcc_make_library(
    opendcc.usd_editor.texture_paint
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/math_utils.h
    ${_src_dir}/texture_paint_entry_point.h
    ${_src_dir}/texture_paint_tool_context.h
    ${_src_dir}/texture_paint_tool_settings.h
    ${_src_dir}/texture_paint_stroke_command.h
    ${_src_dir}/brush_properties.h
    ${_src_dir}/texture_painter.h
    ${_src_dir}/texture_data.h
    CPPFILES
    ${_src_dir}/texture_paint_entry_point.cpp
    ${_src_dir}/texture_paint_tool_context.cpp
    ${_src_dir}/texture_paint_tool_settings.cpp
    ${_src_dir}/texture_paint_stroke_command.cpp
    ${_src_dir}/brush_properties.cpp
    ${_src_dir}/texture_painter.cpp
    ${_src_dir}/texture_data.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    IGL::igl
    Eigen3::Eigen
    VS_FOLDER
    usd/dcc
    PACKAGE_DIR
    opendcc.usd_editor.texture_paint/lib)

opendcc_make_library(
    opendcc.usd_editor.texture_paint.resolver
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/texture_paint_resolver.h
    CPPFILES
    ${_src_dir}/texture_paint_resolver.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    RESOURCES
    ${_src_dir}/plugInfo.json
    RESOURCES_DIR
    packages/opendcc.usd_editor.texture_paint/pxr_plugins/opendcc.usd_editor.texture_paint/resources
    VS_FOLDER
    usd/dcc
    PACKAGE_DIR
    opendcc.usd_editor.texture_paint/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pxr_plugins/plugInfo.json DESTINATION ${PLUGIN_DIR}/pxr_plugins)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

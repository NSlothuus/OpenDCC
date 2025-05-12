set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.point_instancer_tool/)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/usd_editor/point_instancer_tool)
opendcc_make_library(
    opendcc.usd_editor.point_instancer_tool
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/tool_context.h
    ${_src_dir}/tool_settings.h
    ${_src_dir}/entry_point.h
    CPPFILES
    ${_src_dir}/tool_context.cpp
    ${_src_dir}/tool_settings.cpp
    ${_src_dir}/entry_point.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    opendcc.usd_editor.common_tools
    VS_FOLDER
    usd/dcc
    PACKAGE_DIR
    opendcc.usd_editor.point_instancer_tool/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

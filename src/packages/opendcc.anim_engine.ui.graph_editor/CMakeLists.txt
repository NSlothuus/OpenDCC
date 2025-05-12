set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.anim_engine.ui.graph_editor/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/anim_engine/ui/graph_editor)

opendcc_make_library(
    opendcc.anim_engine.ui.graph_editor
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/graph_editor.h
    ${_src_dir}/snap_window.h
    ${_src_dir}/spline_widget.h
    ${_src_dir}/spline_widget_commands.h
    ${_src_dir}/utils.h
    ${_src_dir}/selection_event_dispatcher.h
    ${_src_dir}/channel_editor.h
    CPPFILES
    ${_src_dir}/channel_editor.cpp
    ${_src_dir}/selection_event_dispatcher.cpp
    ${_src_dir}/utils.cpp
    ${_src_dir}/spline_widget_commands.cpp
    ${_src_dir}/graph_editor.cpp
    ${_src_dir}/snap_window.cpp
    ${_src_dir}/spline_widget.cpp
    PRIVATE_DEFINITIONS
    GRAPH_EDITOR_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc.anim_engine.curve
    opendcc.anim_engine.core
    opendcc_lib
    PACKAGE_DIR
    opendcc.anim_engine.ui.graph_editor/lib
    VS_FOLDER
    packages/anim_engine)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

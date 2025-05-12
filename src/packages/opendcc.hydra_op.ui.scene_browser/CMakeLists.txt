set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.ui.scene_browser/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op/ui/scene_browser)
opendcc_make_library(
    opendcc.hydra_op.ui.scene_browser
    QT_AUTOMOC
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/dataSourceTreeWidget.h
    ${_src_dir}/dataSourceValueTreeView.h
    ${_src_dir}/registeredSceneIndexChooser.h
    ${_src_dir}/sceneIndexDebuggerWidget.h
    ${_src_dir}/sceneIndexObserverLoggingTreeView.h
    ${_src_dir}/sceneIndexObserverLoggingWidget.h
    ${_src_dir}/sceneIndexTreeWidget.h
    ${_src_dir}/entry_point.h
    CPPFILES
    ${_src_dir}/dataSourceTreeWidget.cpp
    ${_src_dir}/dataSourceValueTreeView.cpp
    ${_src_dir}/registeredSceneIndexChooser.cpp
    ${_src_dir}/sceneIndexDebuggerWidget.cpp
    ${_src_dir}/sceneIndexObserverLoggingTreeView.cpp
    ${_src_dir}/sceneIndexObserverLoggingWidget.cpp
    ${_src_dir}/sceneIndexTreeWidget.cpp
    ${_src_dir}/entry_point.cpp
    LIBRARY_DEPENDENCIES
    opendcc_lib
    hd
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op.ui.scene_browser/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

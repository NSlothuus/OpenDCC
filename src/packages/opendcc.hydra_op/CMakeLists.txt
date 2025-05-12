set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op)

opendcc_make_library(
    opendcc.hydra_op
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/api.h
    ${_src_dir}/session.h
    PRIVATE_HEADERS
    ${_src_dir}/entry_point.h
    ${_src_dir}/scene_context.h
    ${_src_dir}/select_tool.h
    ${_src_dir}/viewport_ui_extension.h
    ${_src_dir}/scene_index_manager.h
    ${_src_dir}/viewport_render_settings.h
    CPPFILES
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/scene_context.cpp
    ${_src_dir}/select_tool.cpp
    ${_src_dir}/session.cpp
    ${_src_dir}/viewport_ui_extension.cpp
    ${_src_dir}/scene_index_manager.cpp
    ${_src_dir}/viewport_render_settings.cpp
    PRIVATE_DEFINITIONS
    OPENDCC_HYDRA_OP_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc.hydra_op.translator
    opendcc_lib
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op/lib)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/python"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

set(PLUGIN_DIR "${CMAKE_INSTALL_PREFIX}/packages/opendcc.hydra_op.render/")

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/hydra_op/render)

opendcc_make_library(
    opendcc.hydra_op.render
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PRIVATE_HEADERS
    ${_src_dir}/entry_point.h
    ${_src_dir}/hydra_op_render_app_controller.h
    CPPFILES
    ${_src_dir}/entry_point.cpp
    ${_src_dir}/hydra_op_render_app_controller.cpp
    PRIVATE_DEFINITIONS
    OPENDCC_HYDRA_OP_RENDER_EXPORT
    LIBRARY_DEPENDENCIES
    opendcc.hydra_op
    usd_render_app
    VS_FOLDER
    packages/hydra_op
    PACKAGE_DIR
    opendcc.hydra_op.render/lib)

add_subdirectory(bin)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

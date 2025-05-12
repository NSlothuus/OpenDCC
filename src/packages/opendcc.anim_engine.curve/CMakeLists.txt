set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.anim_engine.curve/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/anim_engine/curve)

opendcc_make_library(
    opendcc.anim_engine.curve
    TYPE
    STATIC
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADER
    ${_src_dir}/curve.h
    CPPFILES
    ${_src_dir}/curve.cpp
    LIBRARY_DEPENDENCIES
    opendcc.vendor.animx
    jsoncpp
    VS_FOLDER
    packages/anim_engine
    PACKAGE_DIR
    opendcc.anim_engine.curve/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

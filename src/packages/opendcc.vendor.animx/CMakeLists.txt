set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.vendor.animx/)

set(OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(_src_dir lib/opendcc/vendor/animx)

opendcc_make_library(
    opendcc.vendor.animx
    TYPE
    PLUGIN
    INCLUDE_DIRS
    ${OPENDCC_CURRENT_PACKAGE_INCLUDE_DIR}
    PUBLIC_HEADERS
    ${_src_dir}/animx.h
    ${_src_dir}/internal/nurbs.h
    ${_src_dir}/internal/interpolators.h
    ${_src_dir}/internal/math.h
    ${_src_dir}/internal/T4DblMatrix.h
    ${_src_dir}/internal/T4DblVector.h
    ${_src_dir}/internal/Tbezier.h
    ${_src_dir}/internal/Tquaternion.h
    ${_src_dir}/internal/TrotateXYZ.h
    CPPFILES
    ${_src_dir}/animx.cpp
    PUBLIC_DEFINITIONS
    MAYA_64BIT_TIME_PRECISION
    ANIMX_EXPORT
    VS_FOLDER
    packages/vendor
    PACKAGE_DIR
    opendcc.vendor.animx/lib)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/readme.md DESTINATION ${PLUGIN_DIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

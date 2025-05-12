if(NOT DEFINED KATANA_ROOT AND NOT DEFINED ENV{KATANA_ROOT})
    message(FATAL_ERROR "KATANA_ROOT is not defined")
endif()

# The USD libraries required by the Katana USD Plug-ins
set(USD_LIBRARIES
    ar
    arch
    cameraUtil
    gf
    hd
    hf
    hio
    js
    kind
    ndr
    pcp
    plug
    pxOsd
    sdf
    sdr
    tf
    trace
    usd
    usdGeom
    usdHydra
    usdImaging
    usdLux
    usdRi
    usdShade
    usdSkel
    usdUI
    usdUtils
    usdVol
    vt
    work
    garch
    glf
    usdImagingGL
    usdRender
    hdx
    hdSt
    hgi
    hgiGL
    hgiInterop
    usdMedia
    usdPhysics)

# Auto generated from pxrTargets.cmake from our build using the script below:
# ------------------------------------------------------------------------------
# Provide USD_ROOT as a definition to where the USD you want to print out dependnecies for is available.
# include(${USD_ROOT}/cmake/pxrTargets.cmake) set(USD_LIBS ... ) foreach(usd_lib ${USD_LIBS})
# get_target_property(dep_list ${usd_lib} INTERFACE_LINK_LIBRARIES) message("set(USD_${usd_lib}_DEPENDENCIES
# ${dep_list})") endforeach()
# ------------------------------------------------------------------------------

set(USD_ar_DEPENDENCIES arch;tf;plug;vt)
set(USD_arch_DEPENDENCIES)
set(USD_cameraUtil_DEPENDENCIES tf;gf)
set(USD_gf_DEPENDENCIES arch;tf)
set(USD_hd_DEPENDENCIES plug;tf;trace;vt;work;sdf;cameraUtil;hf;pxOsd)
set(USD_hf_DEPENDENCIES plug;tf;trace)
set(USD_hio_DEPENDENCIES arch;js;plug;tf;vt;trace;ar;hf)
set(USD_js_DEPENDENCIES tf)
set(USD_kind_DEPENDENCIES tf;plug)
set(USD_ndr_DEPENDENCIES tf;plug;vt;work;ar;sdf)
set(USD_pcp_DEPENDENCIES tf;trace;vt;sdf;work;ar)
set(USD_plug_DEPENDENCIES arch;tf;js;trace;work)
set(USD_pxOsd_DEPENDENCIES tf;gf;vt)
set(USD_sdf_DEPENDENCIES arch;tf;gf;trace;vt;work;ar)
set(USD_sdr_DEPENDENCIES tf;vt;ar;ndr;sdf)
set(USD_tf_DEPENDENCIES arch;Boost::python;Python3::Python)
set(USD_trace_DEPENDENCIES arch;js;tf)
set(USD_usd_DEPENDENCIES arch;kind;pcp;sdf;ar;plug;tf;trace;vt;work)
set(USD_usdGeom_DEPENDENCIES js;tf;plug;vt;sdf;trace;usd;work)
set(USD_usdHydra_DEPENDENCIES tf;usd;usdShade)
set(USD_usdImaging_DEPENDENCIES gf;tf;plug;trace;vt;work;hd;pxOsd;sdf;usd;usdGeom;usdLux;usdShade;usdVol;ar)
set(USD_usdLux_DEPENDENCIES tf;vt;ndr;sdf;usd;usdGeom;usdShade)
set(USD_usdRi_DEPENDENCIES tf;vt;sdf;usd;usdShade;usdGeom;usdLux)
set(USD_usdShade_DEPENDENCIES tf;vt;sdf;ndr;sdr;usd;usdGeom)
set(USD_usdSkel_DEPENDENCIES arch;gf;tf;trace;vt;work;sdf;usd;usdGeom)
set(USD_usdUI_DEPENDENCIES tf;vt;sdf;usd)
set(USD_usdUtils_DEPENDENCIES arch;tf;gf;sdf;usd;usdGeom)
set(USD_usdVol_DEPENDENCIES tf;usd;usdGeom)
set(USD_vt_DEPENDENCIES arch;tf;gf;trace)
set(USD_work_DEPENDENCIES tf;trace)

set(USD_garch_DEPENDENCIES arch;tf)
set(USD_glf_DEPENDENCIES ar;arch;garch;gf;hf;hio;plug;tf;trace;sdf)
set(USD_usdImagingGL_DEPENDENCIES
    gf;tf;plug;trace;vt;work;hio;garch;glf;hd;hdx;pxOsd;sdf;sdr;usd;usdGeom;usdHydra;usdShade;usdImaging;ar)
set(USD_usdRender_DEPENDENCIES
    ar;kind;sdf;ndr;sdr;pcp;usd;usdGeom;usdVol;usdMedia;usdShade;usdLux;usdRender;usdHydra;usdRi;usdSkel;usdUI;usdUtils;usdPhysics
)
set(USD_hdx_DEPENDENCIES plug;tf;vt;gf;work;garch;glf;pxOsd;hd;hdSt;hgi;hgiInterop;cameraUtil;sdf)
set(USD_hdSt_DEPENDENCIES hio;garch;glf;hd;hgiGL;hgiInterop;sdr;tf;trace)
set(USD_hgi_DEPENDENCIES gf;plug;tf)
set(USD_hgiGL_DEPENDENCIES arch;garch;hgi;tf;trace)
set(USD_hgiInterop_DEPENDENCIES gf;tf;hgi;vt)
set(USD_usdMedia_DEPENDENCIES tf;vt;sdf;usd;usdGeom)
set(USD_usdPhysics_DEPENDENCIES tf;plug;vt;sdf;trace;usd;usdGeom;usdShade;work)

if(NOT DEFINED USD_LIBRARY_DIR)
    if(NOT DEFINED USD_ROOT)
        message(FATAL_ERROR "Unable to find USD libraries USD_ROOT must be" " specified.")
    endif()
    set(USD_LIBRARY_DIR ${USD_ROOT}/lib)
endif()

if(NOT DEFINED USD_INCLUDE_DIR)
    if(NOT DEFINED USD_ROOT)
        message(FATAL_ERROR "Unable to find USD libraries USD_ROOT must be" " specified.")
    endif()
    set(USD_INCLUDE_DIR ${USD_ROOT}/include)
endif()

set(LIB_EXTENSION "")
if(UNIX AND NOT APPLE)
    set(LIB_EXTENSION .so)
elseif(WIN32)
    set(LIB_EXTENSION .lib)
endif()

foreach(lib ${USD_LIBRARIES})
    set(USD_${lib}_PATH ${USD_LIBRARY_DIR}/${PXR_LIB_PREFIX}${lib}${LIB_EXTENSION})
    if(EXISTS ${USD_${lib}_PATH})
        add_library(${lib} INTERFACE IMPORTED)

        set_target_properties(${lib} PROPERTIES INTERFACE_LINK_LIBRARIES "${USD_${lib}_PATH};${USD_${lib}_DEPENDENCIES}"
                                                INTERFACE_INCLUDE_DIRECTORIES "${USD_INCLUDE_DIR}")

        if(UNIX)
            get_target_property(CURRENT_INCLUDE_DIRS ${lib} INTERFACE_INCLUDE_DIRECTORIES)
            set_target_properties(
                ${lib}
                PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES
                    "${CURRENT_INCLUDE_DIRS};${KATANA_ROOT}/external/foundryboost/include;${KATANA_ROOT}/external/FnUSD/include;${KATANA_ROOT}/external/tbb/include;/usr/local/include/python3.9;/usr/local/include"
            )
        elseif(WIN32)
            get_target_property(CURRENT_INCLUDE_DIRS ${lib} INTERFACE_INCLUDE_DIRECTORIES)
            set_target_properties(
                ${lib}
                PROPERTIES
                    INTERFACE_INCLUDE_DIRECTORIES
                    "${CURRENT_INCLUDE_DIRS};${KATANA_ROOT}/external/foundryboost/include/boost-1_76;${KATANA_ROOT}/external/FnUSD/include;${KATANA_ROOT}/external/tbb/include;${KATANA_ROOT}/bin/include/include"
            )
        endif()
    else()
        message(FATAL_ERROR "Unable to add library ${lib}, " "could not be found in location ${USD_${lib}_PATH}")
    endif()
endforeach()

if(USD_INCLUDE_DIR AND EXISTS "${USD_INCLUDE_DIR}/pxr/pxr.h")
    foreach(_usd_comp MAJOR MINOR PATCH)
        file(STRINGS "${USD_INCLUDE_DIR}/pxr/pxr.h" _usd_tmp REGEX "#define PXR_${_usd_comp}_VERSION .*$")
        string(REGEX MATCHALL "[0-9]+" USD_${_usd_comp}_VERSION ${_usd_tmp})
    endforeach()
    file(STRINGS "${USD_INCLUDE_DIR}/pxr/pxr.h" _usd_tmp REGEX "#define PXR_VERSION .*$")
    string(REGEX MATCHALL "[0-9]+" USD_PXR_VERSION ${_usd_tmp})
    set(USD_VERSION
        ${USD_MAJOR_VERSION}.${USD_MINOR_VERSION}.${USD_PATCH_VERSION}
        CACHE INTERNAL "USD version" FORCE)
endif()

set(KATANA_BIN "${KATANA_ROOT}/bin")

find_program(
    USD_GENSCHEMA_SCRIPT
    NAMES usdGenSchema usdGenSchema.py
    PATHS ${KATANA_BIN}
    DOC "USD Gen schema application" REQUIRED
    NO_DEFAULT_PATH)

set(_run_usd_gen_schema ${USD_GENSCHEMA_SCRIPT})
get_filename_component(USD_GENSCHEMA_DIR ${USD_GENSCHEMA_SCRIPT} DIRECTORY)
if(UNIX)
    list(PREPEND _run_usd_gen_schema ${KATANA_BIN}/python.sh)
elseif(WIN32)
    list(PREPEND _run_usd_gen_schema ${KATANA_BIN}/python.exe)
endif()

set(USD_GENSCHEMA
    ${_run_usd_gen_schema}
    CACHE STRING "" FORCE)

set(PXR_INCLUDE_DIRS ${USD_INCLUDE_DIR})

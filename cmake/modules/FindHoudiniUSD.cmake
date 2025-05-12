# Simple module to find USD.

# can use either ${USD_ROOT}, or ${USD_CONFIG_FILE} (which should be ${USD_ROOT}/pxrConfig.cmake) to find USD, defined
# as either Cmake var or env var
if(NOT DEFINED HOUDINI_ROOT AND NOT DEFINED ENV{HOUDINI_ROOT})
    message(FATAL_ERROR "HOUDINI_ROOT is not defined")
endif()

find_path(
    USD_INCLUDE_DIR pxr/pxr.h
    PATHS ${HOUDINI_ROOT}/toolkit/include $ENV{HOUDINI_ROOT}/include
    DOC "USD Include directory"
    NO_DEFAULT_PATH)

if(WIN32)
    find_path(
        USD_LIBRARY_DIR libpxr_usd.lib
        PATHS ${HOUDINI_ROOT}/custom/houdini/dsolib $ENV{HOUDINI_ROOT}/custom/houdini/dsolib
        DOC "USD Libraries directory")
elseif(UNIX)
    find_path(
        USD_LIBRARY_DIR libpxr_usd.so libusd.dylib
        PATHS ${HOUDINI_ROOT}/custom/houdini/dsolib $ENV{HOUDINI_ROOT}/custom/houdini/dsolib ${HOUDINI_ROOT}/dsolib
              $ENV{HOUDINI_ROOT}/dsolib
        DOC "USD Libraries directory"
        NO_DEFAULT_PATH)
endif()

if(DEFINED HOUDINI_ROOT)
    set(HOUDINI_BIN "${HOUDINI_ROOT}/bin")
else()
    set(HOUDINI_BIN "$ENV{HOUDINI_ROOT}/bin")
endif()

find_program(
    USD_GENSCHEMA_SCRIPT
    NAMES usdGenSchema usdGenSchema.py
    PATHS ${HOUDINI_BIN}
    DOC "USD Gen schema application" REQUIRED
    NO_DEFAULT_PATH)

set(_run_usd_gen_schema ${USD_GENSCHEMA_SCRIPT})

get_filename_component(USD_GENSCHEMA_DIR ${USD_GENSCHEMA_SCRIPT} DIRECTORY)
list(PREPEND _run_usd_gen_schema "${PYTHON_EXECUTABLE}")
set(USD_GENSCHEMA
    ${_run_usd_gen_schema}
    CACHE STRING "" FORCE)

if(USD_INCLUDE_DIR AND EXISTS "${USD_INCLUDE_DIR}/pxr/pxr.h")
    foreach(_usd_comp MAJOR MINOR PATCH)
        file(STRINGS "${USD_INCLUDE_DIR}/pxr/pxr.h" _usd_tmp REGEX "#define PXR_${_usd_comp}_VERSION .*$")
        string(REGEX MATCHALL "[0-9]+" USD_${_usd_comp}_VERSION ${_usd_tmp})
    endforeach()
    file(STRINGS "${USD_INCLUDE_DIR}/pxr/pxr.h" _usd_tmp REGEX "#define PXR_VERSION .*$")
    string(REGEX MATCHALL "[0-9]+" USD_PXR_VERSION ${_usd_tmp})
    set(USD_PXR_VERSION
        ${USD_PXR_VERSION}
        CACHE INTERNAL "" FORCE)
    set(USD_VERSION
        ${USD_MAJOR_VERSION}.${USD_MINOR_VERSION}.${USD_PATCH_VERSION}
        CACHE INTERNAL "USD version" FORCE)
endif()

# find python version which houdini uses
if(WIN32)
    set(_hboost_lib_dir "${HOUDINI_ROOT}/bin")
else()
    set(_hboost_lib_dir "${HOUDINI_ROOT}/dsolib")
endif()
file(
    GLOB _hboost_python_lib_list
    RELATIVE ${_hboost_lib_dir}
    ${_hboost_lib_dir}/*hboost_python*${CMAKE_SHARED_LIBRARY_SUFFIX})
if(NOT _hboost_python_lib_list)
    message(FATAL_ERROR "Failed to find hboost_python library.")
endif()

list(GET _hboost_python_lib_list 0 _hboost_python_lib)
string(REGEX REPLACE ".*hboost_python([0-9]).*" "\\1" _python_major_ver ${_hboost_python_lib})
string(REGEX REPLACE ".*hboost_python[0-9]([0-9]).*" "\\1" _python_minor_ver ${_hboost_python_lib})
set(_python_ver ${_python_major_ver}${_python_minor_ver})

add_library(_houdini_deps INTERFACE)

set(_houdini_libs
    OpenImageIO_sidefx;hboost_filesystem-mt-x64;hboost_iostreams-mt-x64;hboost_system-mt-x64;hboost_regex-mt-x64;hboost_python${_python_ver}-mt-x64
)
foreach(_houdini_lib ${_houdini_libs})
    find_library(
        ${_houdini_lib}_path
        NAMES ${_houdini_lib}
        PATHS ${HOUDINI_ROOT}/dsolib ${HOUDINI_ROOT}/custom/houdini/dsolib/ REQUIRED
        NO_DEFAULT_PATH)

    message(STATUS "Found ${_houdini_lib}: ${${_houdini_lib}_path}")

    target_link_libraries(_houdini_deps INTERFACE ${${_houdini_lib}_path})
endforeach()

find_library(
    _houdini_python_lib
    NAMES python${_python_ver} python${_python_major_ver}.${_python_minor_ver}
          python${_python_major_ver}.${_python_minor_ver}m python
    PATHS "${HOUDINI_ROOT}/python${_python_ver}/libs" "${HOUDINI_ROOT}/python/libs" "${HOUDINI_ROOT}/python/lib"
          REQUIRED
    NO_DEFAULT_PATH)

find_path(
    _houdini_python_include_dir
    NAMES pyconfig.h
    PATHS "${HOUDINI_ROOT}/python${_python_ver}/include" "${HOUDINI_ROOT}/python/include"
          "${HOUDINI_ROOT}/python/include/python${_python_major_ver}.${_python_minor_ver}"
          "${HOUDINI_ROOT}/python/include/python${_python_major_ver}.${_python_minor_ver}m" REQUIRED
    NO_DEFAULT_PATH)

target_link_libraries(_houdini_deps INTERFACE ${_houdini_python_lib})
target_include_directories(_houdini_deps INTERFACE "${_houdini_python_include_dir}")

set(_houdini_pxr_libs
    ar;arch;cameraUtil;garch;gf;glf;hd;hdSt;hdx;hf;hgi;hgiGL;hgiInterop;hio;js;kind;ndr;pcp;plug;pxOsd;sdf;sdr;tf;trace;usd;usdAppUtils;usdGeom;usdHydra;usdImaging;usdImagingGL;usdLux;usdMedia;usdRender;usdRi;usdRiImaging;usdShade;usdSkel;usdSkelImaging;usdUI;usdUtils;usdviewq;usdVol;usdVolImaging;vt;work;
)
foreach(_pxr_lib ${_houdini_pxr_libs})
    find_library(
        ${_pxr_lib}_path
        NAMES libpxr_${_pxr_lib} pxr_${_pxr_lib}
        PATHS ${HOUDINI_ROOT}/custom/houdini/dsolib/ ${HOUDINI_ROOT}/dsolib/ REQUIRED
        NO_DEFAULT_PATH)
    add_library(${_pxr_lib} SHARED IMPORTED)

    set_target_properties(
        ${_pxr_lib}
        PROPERTIES INTERFACE_COMPILE_DEFINITIONS "PXR_PYTHON_ENABLED=1"
                   INTERFACE_INCLUDE_DIRECTORIES "${USD_INCLUDE_DIR}"
                   INTERFACE_LINK_LIBRARIES _houdini_deps
                   IMPORTED_IMPLIB "${${_pxr_lib}_path}"
                   IMPORTED_LOCATION "${${_pxr_lib}_path}")
endforeach()

foreach(_pxr_lib ${_houdini_pxr_libs})
    target_link_libraries(${_pxr_lib} INTERFACE ${_houdini_pxr_libs})
endforeach()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    USD
    REQUIRED_VARS USD_INCLUDE_DIR USD_LIBRARY_DIR USD_GENSCHEMA USD_PXR_VERSION
    VERSION_VAR USD_VERSION)

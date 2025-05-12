# * Try to find CYCLES Once done this will define CYCLES_FOUND - System has CYCLES CYCLES_INCLUDE_DIR - The CYCLES
#   include directories CYCLES_LIBRARIES - The libraries needed to use CYCLES CYCLES_DEFINITIONS - Compiler switches
#   required for using CYCLES Cycles::cycles - imported target with included dir and static libs

set(CYCLES_ROOT
    ""
    CACHE PATH "CYCLES_ROOT")

find_path(
    CYCLES_INCLUDE_DIR
    NAMES atomic_ops.h
    HINTS ${CYCLES_ROOT}/include $ENV{CYCLES_ROOT}/include
    DOC "Cycles include path")

# Warning: due to cycles is a static lib, order is important!
set(_cycles_libs
    cycles_session
    cycles_scene
    cycles_bvh
    cycles_integrator
    cycles_subd
    cycles_util
    cycles_device
    cycles_kernel_osl
    cycles_graph
    cycles_kernel
    extern_cuew
    extern_hipew
    extern_sky)

set(CYCLES_LIBRARIES "")

foreach(_lib ${_cycles_libs})
    find_library(
        ${_lib}_LIBRARY
        NAMES ${_lib}
        HINTS ${CYCLES_ROOT}
        PATH_SUFFIXES lib)
    if(${_lib}_LIBRARY)
        add_library(Cycles::${_lib} STATIC IMPORTED GLOBAL)
        set_target_properties(Cycles::${_lib} PROPERTIES IMPORTED_LOCATION "${${_lib}_LIBRARY}")
        list(APPEND CYCLES_LIBRARIES Cycles::${_lib})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Cycles
    REQUIRED_VARS
        CYCLES_INCLUDE_DIR
        cycles_bvh_LIBRARY
        cycles_device_LIBRARY
        cycles_graph_LIBRARY
        cycles_integrator_LIBRARY
        cycles_kernel_LIBRARY
        cycles_kernel_osl_LIBRARY
        cycles_scene_LIBRARY
        cycles_session_LIBRARY
        cycles_subd_LIBRARY
        cycles_util_LIBRARY
        extern_cuew_LIBRARY
        extern_hipew_LIBRARY
        extern_sky_LIBRARY)

if(NOT TARGET Cycles::cycles)
    add_library(Cycles::cycles INTERFACE IMPORTED)
    set_target_properties(Cycles::cycles PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CYCLES_INCLUDE_DIR}"
                                                    INTERFACE_LINK_LIBRARIES "${CYCLES_LIBRARIES}")
endif()

#
# In: OSL_ROOT
#
# Out: OSL_FOUND OSL_LIBRARY_DIR OSL_INCLUDE_DIR OSL_LIBRARIES OSL_DEFINITIONS OSL_BASE

set(OSL_ROOT
    ""
    CACHE PATH "OSL ROOT")

find_program(
    OSL_BINARY OSL
    PATHS ${OSL_ROOT}/bin
    NO_SYSTEM_ENVIRONMENT_PATH)

if(OSL_BINARY STREQUAL "OSL_BINARY-NOTFOUND")
    find_program(
        OSL_BINARY oslc
        PATHS ${OSL_ROOT}/bin
        NO_SYSTEM_ENVIRONMENT_PATH)
endif()

if(OSL_BINARY STREQUAL "OSL_BINARY-NOTFOUND")
    set(OSL_FOUND FALSE)
else()
    set(OSL_FOUND TRUE)
    get_filename_component(OSL_BASE ${OSL_BINARY} DIRECTORY)
    get_filename_component(OSL_BASE ${OSL_BASE} DIRECTORY)

    if(UNIX)
        set(LIBDIR_ARCH 64)
    endif()
    set(OSL_LIBRARY_DIR
        ${OSL_BASE}/lib${LIBDIR_ARCH}
        CACHE PATH "Library directory" FORCE)
    set(OSL_INCLUDE_DIR
        ${OSL_BASE}/include
        CACHE PATH "Include directory" FORCE)

    # set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

    # OSL
    foreach(_lib libtestshade osl.imageio oslcomp oslexec oslnoise oslquery)

        find_library(OSL_${_lib}_LIBRARY ${_lib} PATHS ${OSL_LIBRARY_DIR})
        list(APPEND OSL_LIBRARIES ${OSL_${_lib}_LIBRARY})

    endforeach(_lib)

    set(OSL_DEFINITIONS "") # why again

endif()

if(OSL_FIND_REQUIRED AND NOT OSL_FOUND)
    message(FATAL_ERROR "Could not find OSL")
endif()

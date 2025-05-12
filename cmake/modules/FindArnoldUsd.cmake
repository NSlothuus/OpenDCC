#
# In: ARNOLDUSD_ROOT
#
# Out: ARNOLDUSD_FOUND ARNOLDUSD_LIBRARY_DIR ARNOLDUSD_INCLUDE_DIR ARNOLDUSD_LIBRARY ARNOLDUSD_NDR_LIBRARY
# ARNOLDUSD_DEFINITIONS

set(ARNOLDUSD_ROOT
    ""
    CACHE PATH "ARNOLDUSD ROOT")

if(ARNOLDUSD_BINARY STREQUAL "ARNOLDUSD_BINARY-NOTFOUND")
    find_program(
        ARNOLDUSD_BINARY arnold_to_usd
        PATHS ${ARNOLDUSD_ROOT}/bin
        NO_SYSTEM_ENVIRONMENT_PATH)
endif()

if(ARNOLDUSD_ROOT STREQUAL "")
    set(ARNOLDUSD_FOUND FALSE)
else()
    set(ARNOLDUSD_FOUND TRUE)

    set(ARNOLDUSD_LIBRARY_DIR
        ${ARNOLDUSD_ROOT}/lib
        CACHE PATH "Library directory" FORCE)
    find_path(
        ARNOLDUSD_INCLUDE_DIR arnold_usd.h
        PATHS ${ARNOLDUSD_ROOT}/include $ENV{ARNOLD_ROOT}/include ${ARNOLDUSD_ROOT}/include/arnold_usd
              $ENV{ARNOLD_ROOT}/include/arnold_usd
        DOC "Arnold include path")
    if(ARNOLDUSD_INCLUDE_DIR STREQUAL "ARNOLDUSD_INCLUDE_DIR-NOTFOUND")
        set(ARNOLDUSD_INCLUDE_DIR ${ARNOLDUSD_ROOT}/include/arnold_usd)
    endif()

    # set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")

    # ARNOLDUSD
    find_library(
        ARNOLDUSD_LIBRARY
        NAMES usdArnold libusdArnold
        PATHS ${ARNOLDUSD_LIBRARY_DIR})
    find_library(
        ARNOLDUSD_NDR_LIBRARY
        NAMES ndrArnold ndrArnold${CMAKE_SHARED_LIBRARY_SUFFIX}
        PATHS ${ARNOLDUSD_ROOT}/plugin)
    set(ARNOLDUSD_DEFINITIONS "") # why again
endif()

if(ARNOLDUSD_FIND_REQUIRED AND NOT ARNOLDUSD_FOUND)
    message(FATAL_ERROR "Could not find ARNOLDUSD")
endif()

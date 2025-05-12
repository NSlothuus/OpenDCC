# * Try to find ZMQ Once done this will define ZMQ_FOUND - System has ZMQ ZMQ_INCLUDE_DIRS - The ZMQ include directories
#   ZMQ_LIBRARIES - The libraries needed to use ZMQ ZMQ_DEFINITIONS - Compiler switches required for using ZMQ

set(ZMQ_ROOT
    ""
    CACHE PATH "ZMQ_ROOT")
if(WIN32)
    find_library(
        ZMQ_LIBRARY
        NAMES libzmq
        PATHS ${ZMQ_ROOT}/lib $ENV{ZMQ_ROOT}/lib
        DOC "ZeroMQ library")
else()
    find_library(
        ZMQ_LIBRARY
        NAMES zmq
        PATHS ${ZMQ_ROOT}/bin $ENV{ZMQ_ROOT}/bin
        DOC "ZeroMQ library")
endif()

find_path(
    ZMQ_INCLUDE_DIR zmq.h
    PATHS ${ZMQ_ROOT}/include $ENV{ZMQ_ROOT}/include
    DOC "ZeroMQ include path")

set(ZMQ_LIBRARIES ${ZMQ_LIBRARY})
set(ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set ZMQ_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(ZMQ DEFAULT_MSG ZMQ_LIBRARY ZMQ_INCLUDE_DIR)

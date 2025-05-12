# Locate OpenSubdiv library This module defines OpenSubdiv_FOUND OPENSUBDIV_LIBRARIES OPENSUBDIV_CPU_LIBRARY
# OPENSUBDIV_GPU_LIBRARY OPENSUBDIV_INCLUDE_DIR
#
find_path(
    OPENSUBDIV_INCLUDE_DIR opensubdiv/version.h
    HINTS $ENV{OPENSUBDIV_DIR} ${OPENSUBDIV_DIR}
    PATH_SUFFIXES include/
    PATHS ~/Library/Frameworks
          /Library/Frameworks
          /sw # Fink
          /opt/local # DarwinPorts
          /opt/csw # Blastwave
          /opt)

find_library(
    OPENSUBDIV_GPU_LIBRARY
    NAMES osdGPU
    HINTS $ENV{OPENSUBDIV_DIR} ${OPENSUBDIV_DIR}
    PATH_SUFFIXES lib64 lib
    PATHS ~/Library/Frameworks /Library/Frameworks /sw /opt/local /opt/csw /opt)

find_library(
    OPENSUBDIV_CPU_LIBRARY
    NAMES osdCPU
    HINTS $ENV{OPENSUBDIV_DIR} ${OPENSUBDIV_DIR}
    PATH_SUFFIXES lib64 lib
    PATHS ~/Library/Frameworks /Library/Frameworks /sw /opt/local /opt/csw /opt)

if(OPENSUBDIV_CPU_LIBRARY OR OPENSUBDIV_GPU_LIBRARY)
    if(OPENSUBDIV_CPU_LIBRARY AND OPENSUBDIV_GPU_LIBRARY)
        set(OSD_LIBS ${OPENSUBDIV_CPU_LIBRARY};${OPENSUBDIV_GPU_LIBRARY})
    elseif(OPENSUBDIV_CPU_LIBRARY)
        set(OSD_LIBS ${OPENSUBDIV_CPU_LIBRARY})
    elseif(OPENSUBDIV_GPU_LIBRARY)
        set(OSD_LIBS ${OPENSUBDIV_GPU_LIBRARY})
    endif()
    set(OPENSUBDIV_LIBRARIES
        "${OSD_LIBS}"
        CACHE STRING "OpenSubdiv Libraries")
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set OpenSubdiv_FOUND to TRUE if all listed variables are TRUE
find_package_handle_standard_args(OpenSubdiv REQUIRED_VARS OPENSUBDIV_LIBRARIES OPENSUBDIV_INCLUDE_DIR)

mark_as_advanced(OPENSUBDIV_INCLUDE_DIR OPENSUBDIV_GPU_LIBRARY OPENSUBDIV_CPU_LIBRARY)

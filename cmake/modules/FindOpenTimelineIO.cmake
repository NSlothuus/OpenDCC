if(NOT DEFINED OTIO_ROOT)
    message(FATAL_ERROR "OTIO_ROOT is not defined.")
endif()

find_path(OTIO_INCLUDE_DIR opentime/timeRange.h PATHS ${OTIO_ROOT}/include)

set(OTIO_INCLUDE_DIRS
    "${OTIO_INCLUDE_DIR};${OTIO_INCLUDE_DIR}/opentime;${OTIO_INCLUDE_DIR}/opentimelineio;${OTIO_INCLUDE_DIR}/opentimelineio/deps"
)

set(_otio_libs opentime opentimelineio)
set(OTIO_LIBRARIES "")

foreach(_lib ${_otio_libs})
    find_library(
        ${_lib}_LIBRARY
        NAMES ${_lib}
        HINTS ${OTIO_ROOT}/lib ${OTIO_ROOT}/lib/site-package/opentimelineio ${OTIO_ROOT}/bin)
    if(${_lib}_LIBRARY)
        add_library(OTIO::${_lib} UNKNOWN IMPORTED)
        set_target_properties(OTIO::${_lib} PROPERTIES IMPORTED_LOCATION "${${_lib}_LIBRARY}")
        list(APPEND OTIO_LIBRARIES OTIO::${_lib})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenTimelineIO REQUIRED_VARS OTIO_INCLUDE_DIR opentime_LIBRARY opentimelineio_LIBRARY)

if(NOT TARGET OTIO::otio)
    add_library(OTIO::otio INTERFACE IMPORTED)
    set_target_properties(OTIO::otio PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${OTIO_INCLUDE_DIRS}"
                                                INTERFACE_LINK_LIBRARIES "${OTIO_LIBRARIES}")
endif()

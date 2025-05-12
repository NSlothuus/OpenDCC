if(NOT DEFINED SKIA_ROOT)
    message(FATAL_ERROR "SKIA_ROOT is not defined.")
endif()

set(SKIA_INCLUDE_DIR ${SKIA_ROOT})

set(_skia_libs
    skcms
    skia
    skottie
    skparagraph
    skresources
    sksg
    skshaper
    skunicode)
set(SKIA_LIBRARIES "")

foreach(_lib ${_skia_libs})
    find_library(
        ${_lib}_LIBRARY
        NAMES ${_lib}
        HINTS ${SKIA_ROOT}/out/Release-x64)
    if(${_lib}_LIBRARY)
        add_library(Skia::${_lib} UNKNOWN IMPORTED)
        set_target_properties(Skia::${_lib} PROPERTIES IMPORTED_LOCATION "${${_lib}_LIBRARY}")
        list(APPEND SKIA_LIBRARIES Skia::${_lib})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Skia REQUIRED_VARS SKIA_INCLUDE_DIR SKIA_LIBRARIES)

if(NOT TARGET Skia::Skia)
    add_library(Skia::Skia INTERFACE IMPORTED)
    set_target_properties(Skia::Skia PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SKIA_INCLUDE_DIR}"
                                                INTERFACE_LINK_LIBRARIES "${SKIA_LIBRARIES}")
endif()

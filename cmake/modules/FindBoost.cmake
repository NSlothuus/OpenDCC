if(DCC_HOUDINI_SUPPORT)
    set(Boost_NAMESPACE "hboost")
    set(Boost_INCLUDE_DIR
        "${HOUDINI_ROOT}/toolkit/include"
        CACHE PATH "")
    if(WIN32)
        set(Boost_LIBRARY_DIR
            "${HOUDINI_ROOT}/custom/houdini/dsolib"
            CACHE PATH "")
    else()
        set(Boost_LIBRARY_DIR
            "${HOUDINI_ROOT}/dsolib"
            CACHE PATH "")
    endif()
    set(Boost_LIB_PREFIX "")
    include("${CMAKE_CURRENT_LIST_DIR}/FindHoudiniBoost.cmake")
else()
    include("${CMAKE_ROOT}/Modules/FindBoost.cmake")
endif()

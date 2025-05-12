find_path(
    BULLET_INCLUDE_DIR
    NAMES btBulletCollisionCommon.h
    HINTS ${BULLET_ROOT}/include
    PATH_SUFFIXES bullet)

set(_bullet_libs BulletDynamics BulletCollision BulletRobotics LinearMath)
set(BULLET_LIBRARIES "")

foreach(_lib ${_bullet_libs})
    find_library(
        ${_lib}_LIBRARY
        NAMES ${_lib} ${_lib}_Release ${_lib}_RelWithDebugInfo ${_lib}_Debug
        HINTS ${BULLET_ROOT}
        PATH_SUFFIXES lib)
    if(${_lib}_LIBRARY)
        add_library(Bullet3::${_lib} STATIC IMPORTED GLOBAL)
        set_target_properties(Bullet3::${_lib} PROPERTIES IMPORTED_LOCATION "${${_lib}_LIBRARY}")
        list(APPEND BULLET_LIBRARIES Bullet3::${_lib})
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Bullet3 REQUIRED_VARS BULLET_INCLUDE_DIR BulletCollision_LIBRARY BulletDynamics_LIBRARY BulletRobotics_LIBRARY
                          LinearMath_LIBRARY)

if(NOT TARGET Bullet3::bullet)
    add_library(Bullet3::bullet INTERFACE IMPORTED)
    set_target_properties(
        Bullet3::bullet PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${BULLET_ROOT}/include;${BULLET_INCLUDE_DIR}"
                                   INTERFACE_LINK_LIBRARIES "${BULLET_LIBRARIES}")
endif()

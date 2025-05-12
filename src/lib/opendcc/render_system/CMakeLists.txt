get_filename_component(SOURCE_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)

include_directories(${TBB_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_INCLUDE_DIR} ${SOURCE_PARENT_DIR})

set(include api.h irender.h render_factory.h render_system.h)

set(src render_factory.cpp render_system.cpp)

add_library(render_system SHARED ${src} ${include})

if(NOT MSVC)
    target_link_libraries(render_system PRIVATE ${CMAKE_DL_LIBS})
endif()
target_compile_definitions(render_system PRIVATE -DOPENDCC_RENDER_SYSTEM_EXPORT)

install(FILES ${include} DESTINATION "include/opendcc/render_system/")

install(
    TARGETS render_system
    EXPORT render_system-targets
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    RUNTIME DESTINATION "bin")

install(
    EXPORT render_system-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/render_system
    NAMESPACE opendcc::)

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:render_system>
        DESTINATION bin
        OPTIONAL)
endif()

include_directories(${PROJECT_SOURCE_DIR}/src)

set(src_py bind.cpp)

source_group("Source" FILES ${src_py})
add_library(render_system_py SHARED ${src_py})

target_link_libraries(render_system_py render_system ${PYTHON_LIBRARY} pybind11::pybind11)
if(MSVC)
    set_target_properties(render_system_py PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(render_system_py PROPERTIES SUFFIX ".so")
endif()
set_target_properties(render_system_py PROPERTIES PREFIX "" OUTPUT_NAME rendersystem)
install(
    TARGETS render_system_py
    RUNTIME DESTINATION "${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc"
    LIBRARY DESTINATION "${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc")

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:render_system_py>
        DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc
        OPTIONAL)
endif()

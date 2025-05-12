include_directories(${ZMQ_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src/lib)
add_definitions(-DOPENDCC_RENDER_VIEW_EXPORT)
add_library(render_view_display_driver_api SHARED api.h display_driver_api.cpp display_driver_api.h)
make_folder("render_view" render_view_display_driver_api)
target_link_libraries(render_view_display_driver_api ${ZMQ_LIBRARIES})

install(
    TARGETS render_view_display_driver_api
    EXPORT render_view_display_driver_api-targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

install(FILES api.h display_driver_api.h DESTINATION "include/opendcc/render_view/display_driver_api")
install(
    EXPORT render_view_display_driver_api-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/render_view_display_driver_api
    NAMESPACE opendcc::)
if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:render_view_display_driver_api>
        DESTINATION bin
        OPTIONAL)
endif()

set(include render_settings_registry.h render_settings_property_factory.h)

set(src render_settings_registry.cpp render_settings_property_factory.cpp)

add_library(render_settings_property_proxy SHARED ${include} ${src} plugInfo.json.in)

target_include_directories(
    render_settings_property_proxy PUBLIC ${PROJECT_SOURCE_DIR}/src/lib ${PROJECT_BINARY_DIR}/include
                                          ${PROJECT_SOURCE_DIR} ${USD_INCLUDE_DIR})

target_link_libraries(render_settings_property_proxy hydra_render_session_api usd_fallback_proxy_utils
                      usd_fallback_proxy usdImaging usd)

if(WIN32)
    file(RELATIVE_PATH LIBRARY_LOCATION
         ${CMAKE_INSTALL_PREFIX}/plugin/opendcc/render_settings_property_proxy/resources/ ${CMAKE_INSTALL_PREFIX}/bin)
else()
    file(RELATIVE_PATH LIBRARY_LOCATION
         ${CMAKE_INSTALL_PREFIX}/plugin/opendcc/render_settings_property_proxy/resources/
         ${CMAKE_INSTALL_PREFIX}/plugin/opendcc)
endif()

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json.in" "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json
        DESTINATION ${CMAKE_INSTALL_PREFIX}/plugin/opendcc/render_settings_property_proxy/resources/)

install(FILES ${header} DESTINATION "include/render_settings_property_proxy/")

if(WIN32)
    install(
        TARGETS render_settings_property_proxy
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "lib"
        LIBRARY DESTINATION "lib")
else()
    install(
        TARGETS render_settings_property_proxy
        RUNTIME DESTINATION "bin"
        ARCHIVE DESTINATION "plugin/opendcc"
        LIBRARY DESTINATION "plugin/opendcc")
endif()

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:render_settings_property_proxy>
        DESTINATION bin
        OPTIONAL)
endif()

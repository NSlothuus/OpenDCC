set(PROJECT_NAME moonray_property_proxy)

set(HEADERS moonray_property_factory.h)

set(SRC moonray_property_factory.cpp)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SRC})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/lib/ ${USD_FALLBACK_PROXY_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} usd_fallback_proxy usd_fallback_proxy_utils usdShade)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION plugin/moonray
    LIBRARY DESTINATION plugin/moonray
    ARCHIVE DESTINATION plugin/moonray)
file(RELATIVE_PATH LIBRARY_LOCATION ${CMAKE_INSTALL_PREFIX}/plugin/moonray/${PROJECT_NAME}/resources/..
     ${CMAKE_INSTALL_PREFIX}/plugin/moonray)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json.in" "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json
        DESTINATION ${CMAKE_INSTALL_PREFIX}/plugin/moonray/${PROJECT_NAME}/resources/)
# install(CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/lib/moonray/plugInfo.json\" \"{\n    \\\"Includes\\\": [
# \\\"*/resources/\\\" ]\n}\")" )
if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
        DESTINATION plugin/moonray
        OPTIONAL)
endif()

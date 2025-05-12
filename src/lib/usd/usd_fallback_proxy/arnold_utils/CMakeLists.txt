add_definitions(-DNOMINMAX)
set(PROJECT_NAME arnold_utils)

set(HEADERS api.h utils.h)

set(SRC utils.cpp)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SRC})

target_compile_definitions(${PROJECT_NAME} PRIVATE -DARNOLD_UTILS_EXPORT)

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/lib/ ${USD_FALLBACK_PROXY_INCLUDE_DIR}
                                                  ${ARNOLD_INCLUDE_DIR} ${ARNOLDUSD_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${ARNOLDUSD_NDR_LIBRARY} ${ARNOLD_LIBRARY} ndr sdf)

install(
    TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION plugin/usd
    ARCHIVE DESTINATION plugin/usd)

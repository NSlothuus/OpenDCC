set(headers api.h utils.h)

set(src utils.cpp)

add_library(usd_fallback_proxy_utils SHARED ${headers} ${src})

target_compile_definitions(usd_fallback_proxy_utils PRIVATE -DFALLBACK_PROXY_UTILS_EXPORT)

target_include_directories(usd_fallback_proxy_utils PUBLIC ${USD_INCLUDE_DIR})

target_link_libraries(usd_fallback_proxy_utils usd usdRender usd_fallback_proxy)

install(
    TARGETS usd_fallback_proxy_utils
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION plugin/usd
    ARCHIVE DESTINATION plugin/usd)

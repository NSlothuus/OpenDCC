set(USD_FALLBACK_PROXY_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
    CACHE INTERNAL "Usd Fallback Proxy")
add_subdirectory(usd_fallback_proxy)
add_subdirectory(usd_ui_ext)

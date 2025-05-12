
add_subdirectory(core)

add_subdirectory(py_usd_fallback_proxy)
add_subdirectory(render_settings)

add_subdirectory(schema_ext)

add_subdirectory(storm_render_settings)
add_subdirectory(utils)

if(DCC_USD_FALLBACK_PROXY_BUILD_ARNOLD_USD)
    add_subdirectory(arnold_render_settings)
    add_subdirectory(arnold_usd)
    make_folder("usd/fallback_proxy" arnold_usd_property_proxy)
endif()

if(DCC_BUILD_ARNOLD_SUPPORT OR DCC_USD_FALLBACK_PROXY_BUILD_ARNOLD_USD)
    add_subdirectory(arnold_utils)
    make_folder("usd/fallback_proxy" arnold_utils)
endif()

if(DCC_USD_FALLBACK_PROXY_BUILD_CYCLES)
    add_subdirectory(cycles)
    add_subdirectory(cycles_render_settings)
    make_folder("usd/fallback_proxy" cycles_property_proxy ndrCycles)
endif()

if(DCC_HOUDINI_SUPPORT)
    add_subdirectory(karma)
    make_folder("usd/fallback_proxy" karma_property_proxy)
endif()

if(DCC_USD_FALLBACK_PROXY_BUILD_MOONRAY)
    add_subdirectory(moonray)
    add_subdirectory(moonray_render_settings)
    make_folder("usd/fallback_proxy" moonray_property_proxy)
endif()

if(DCC_BUILD_RENDERMAN_SUPPORT)
    add_subdirectory(renderman)
    add_subdirectory(renderman_render_settings)
    make_folder("usd/fallback_proxy" renderman_property_proxy)
endif()

make_folder("usd/fallback_proxy" usd_fallback_proxy py_usd_fallback_proxy render_settings_property_proxy schema_ext
            usd_fallback_proxy_utils)

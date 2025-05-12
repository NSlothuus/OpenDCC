add_subdirectory(opendcc)

add_subdirectory(usd)

install(
    CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/usd/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
)
if(DCC_USD_FALLBACK_PROXY_BUILD_CYCLES)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/cycles/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
    )
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/cycles/renderer_info.json\"
\"{\n\
    \\\"HdCyclesPlugin\\\": {\n\
        \\\"depth_style\\\": \\\"linear\\\"\n\
    }
}\")")
endif()

if(DCC_BUILD_ARNOLD_SUPPORT
   OR DCC_USD_FALLBACK_PROXY_BUILD_ARNOLD_USD)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/arnold/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
    )
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/arnold/renderer_info.json\"
    \"{\n\
        \\\"HdArnoldRendererPlugin\\\": {\n\
            \\\"depth_style\\\": \\\"opengl\\\"\n\
        }
}\")")
endif()

if(DCC_BUILD_RENDERMAN_SUPPORT)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/renderman/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
    )
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/renderman/renderer_info.json\"
\"{\n\
    \\\"HdPrmanLoaderRendererPlugin\\\": {\n\
        \\\"depth_style\\\": \\\"ndc\\\"\n\
    }
}\")")
endif()

if(UNIX AND DCC_USD_FALLBACK_PROXY_BUILD_MOONRAY)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/moonray/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
    )
endif()

if(DCC_HOUDINI_SUPPORT)
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/karma/plugInfo.json\" \"{\n    \\\"Includes\\\": [ \\\"*/resources/\\\" ]\n}\")"
    )
    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/plugin/karma/renderer_info.json\"
\"{\n\
    \\\"BRAY_HdKarma\\\": {\n\
        \\\"depth_style\\\": \\\"linear\\\"\n\
    }
}\")")
endif()

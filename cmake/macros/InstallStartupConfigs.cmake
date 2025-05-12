function(install_config CONFIG)
    if("${DCC_DEFAULT_CONFIG}" STREQUAL "${CONFIG}")
        set(_new_name "default.toml")
    else()
        set(_new_name "${CONFIG}")
    endif()
    configure_file("${PROJECT_SOURCE_DIR}/configs/${CONFIG}.in" "${PROJECT_BINARY_DIR}/configs/${_new_name}")

    # install only cmake explicitly defined configs
    foreach(INSTALL_CONFIG ${DCC_DEFAULT_CONFIG} ${DCC_ADDITIONAL_CONFIGS})
        if(${INSTALL_CONFIG} STREQUAL ${CONFIG})
            install(FILES "${PROJECT_BINARY_DIR}/configs/${_new_name}" DESTINATION configs)
        endif()
    endforeach()

endfunction()

include(generate_startup_scripts)

file(GLOB DCC_ALL_CONFIGS "${PROJECT_SOURCE_DIR}/configs/*.in")
# configure all possible configs since that used in some CI build i.e. swap config and make distrib without rebuild
foreach(CONFIG_FILE_PATH ${DCC_ALL_CONFIGS})
    get_filename_component(CONFIG ${CONFIG_FILE_PATH} NAME_WLE)
    install_config(${CONFIG})
endforeach()

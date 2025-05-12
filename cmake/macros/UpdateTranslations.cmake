function(setup_update_translations OUT_VAR)
    set(ts_files "${CMAKE_SOURCE_DIR}/i18n/i18n.en.ts")

    set(Qt_PATH "${_qt5Core_install_prefix}/bin")

    get_target_property(pyside_dir Shiboken2::shiboken2 IMPORTED_LOCATION_RELEASE)
    if(NOT pyside_dir)
        get_target_property(pyside_dir Shiboken2::shiboken2 IMPORTED_LOCATION_RELWITHDEBINFO)
    endif()
    get_filename_component(pyside_dir ${pyside_dir} DIRECTORY)

    if(WIN32)
        set(OS_ENV_SEPARATOR ";")
        set(OS_LIBRARY_ENV_NAME "PATH")
    else()
        set(OS_ENV_SEPARATOR ":")
        set(OS_LIBRARY_ENV_NAME "LD_LIBRARY_PATH")
    endif()

    add_custom_target(
        update_translations
        DEPENDS "${CMAKE_SOURCE_DIR}/cmake/macros/make_i18n.py"
        COMMAND
            ${CMAKE_COMMAND} -E env
            "${OS_LIBRARY_ENV_NAME}=${Qt_PATH}${OS_ENV_SEPARATOR}${pyside_dir}${OS_ENV_SEPARATOR}$ENV{${OS_LIBRARY_ENV_NAME}}"
            "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/cmake/macros/make_i18n.py" make_ts --src_dir
            "${CMAKE_SOURCE_DIR}/src" --output_dir "${CMAKE_SOURCE_DIR}/i18n" --qt_path "${_qt5Core_install_prefix}"
            --lang ${DCC_LANG})

    if(${DCC_LANG} STREQUAL "all")
        set(qm_files "${CMAKE_CURRENT_BINARY_DIR}/i18n/i18n.en.qm")
    else()
        set(qm_files "${CMAKE_CURRENT_BINARY_DIR}/i18n/i18n.${DCC_LANG}.qm")
    endif()

    add_custom_command(
        OUTPUT ${qm_files}
        DEPENDS ${ts_files} "${CMAKE_SOURCE_DIR}/cmake/macros/make_i18n.py"
        COMMAND
            ${CMAKE_COMMAND} -E env
            "${OS_LIBRARY_ENV_NAME}=${Qt_PATH}${OS_ENV_SEPARATOR}${pyside_dir}${OS_ENV_SEPARATOR}$ENV{${OS_LIBRARY_ENV_NAME}}"
            "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/cmake/macros/make_i18n.py" make_qm --qt_path
            "${_qt5Core_install_prefix}" --inputs ${ts_files} --output_dir "${CMAKE_CURRENT_BINARY_DIR}/i18n")

    install(FILES ${qm_files} DESTINATION i18n)

    foreach(_f ${qm_files})
        string(REGEX MATCHALL "^.*\\.(.*)\\.qm$" _ ${_f})
        file(
            GLOB _qt_qms
            LIST_DIRECTORIES false
            "${_qt5Core_install_prefix}/translations/qt*_${CMAKE_MATCH_1}.qm")
        set(_lang ${CMAKE_MATCH_1})
        install(
            FILES "${_qt5Core_install_prefix}/translations/qt_${_lang}.qm"
                  "${_qt5Core_install_prefix}/translations/qtbase_${_lang}.qm"
                  "${_qt5Core_install_prefix}/translations/qtscript_${_lang}.qm"
                  "${_qt5Core_install_prefix}/translations/qtmultimedia_${_lang}.qm"
                  "${_qt5Core_install_prefix}/translations/qtxmlpatterns_${_lang}.qm"
            DESTINATION i18n)
    endforeach()

    # Return qm_files to caller
    set(${OUT_VAR}
        ${qm_files}
        PARENT_SCOPE)
endfunction()

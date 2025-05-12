option(DCC_INSTALL_QT5 "" OFF)
option(DCC_INSTALL_ADS "" ON)
option(DCC_INSTALL_PYTHON "" OFF)
option(DCC_INSTALL_PYTHON_DEPS "" OFF)
option(DCC_INSTALL_PYSIDE2 "" OFF)
option(DCC_INSTALL_BOOST "" OFF)
option(DCC_INSTALL_ILMBASE "" OFF)
option(DCC_INSTALL_OCIO "" OFF)
option(DCC_INSTALL_OCIO_DEFAULT_CONFIG "" OFF)
option(DCC_INSTALL_MTLX_DEFAULT_CONFIG "" OFF)
option(DCC_INSTALL_OIIO "" OFF)
option(DCC_INSTALL_LUA "" OFF)
option(DCC_INSTALL_LUA_DEPS "" OFF)
option(DCC_INSTALL_OSL "" OFF)
option(DCC_INSTALL_OPENVDB "" OFF)

option(DCC_INSTALL_TBB "" OFF)
option(DCC_INSTALL_GLEW "" OFF)
option(DCC_INSTALL_SENTRY "" ON)
option(DCC_INSTALL_ZMQ "" OFF)

option(DCC_INSTALL_USD "" OFF)

# used for usdabc for now
option(DCC_INSTALL_ALEMBIC "" OFF)
# used for hdembree for now
option(DCC_INSTALL_EMBREE "" OFF)
# if OIIO with ptex
option(DCC_INSTALL_PTEX "" OFF)

# in case opensubdiv is dynamic
option(DCC_INSTALL_OPENSUBDIV "" OFF)
# OSL DEP
option(DCC_INSTALL_PARTIO "" OFF)

option(DCC_INSTALL_ARNOLD "" OFF)

option(DCC_INSTALL_GRAPHVIZ "" OFF)

function(install_qt5_plugin _qt_plugin_name _qt_plugins_var)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
    get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
    get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
    set(_qt_plugin_dest "qt-plugins/${_qt_plugin_type}")
    install(
        FILES "${_qt_plugin_path}"
        DESTINATION "${_qt_plugin_dest}"
        ${COMPONENT})
endfunction()

# install linux shell script to setup runtime env
if(NOT WIN32)
    if(DCC_HOUDINI_SUPPORT)
        set(PYTHON_MAJOR_MINOR "${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")

        configure_file("${PROJECT_SOURCE_DIR}/cmake/macros/houdini_opendcc_setup.sh.in"
                       "${CMAKE_INSTALL_PREFIX}/houdini_opendcc_setup.sh" @ONLY)
    elseif(DCC_KATANA_SUPPORT)
        install(FILES ${PROJECT_SOURCE_DIR}/cmake/macros/katana_opendcc_setup.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
    else()
        if("${DCC_DEFAULT_CONFIG}" STREQUAL "anim3dplatform-usd_editor.toml")
            install(
                FILES ${PROJECT_SOURCE_DIR}/cmake/macros/opendcc_setup.sh
                DESTINATION ${CMAKE_INSTALL_PREFIX}
                RENAME anim3dplatform_dcc_setup.sh)
        else()
            install(FILES ${PROJECT_SOURCE_DIR}/cmake/macros/opendcc_setup.sh DESTINATION ${CMAKE_INSTALL_PREFIX})
        endif()
    endif()
endif()

if(DCC_INSTALL_QT5)
    list(
        APPEND
        QT_DEPS
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Network)

    if(WIN32)
        install_qt5_plugin(Qt5::QWindowsIntegrationPlugin QT_PLUGINS)
    elseif(APPLE)
        install_qt5_plugin(Qt5::QCocoaIntegrationPlugin QT_PLUGINS)
        install_qt5_plugin(Qt5::QOffscreenIntegrationPlugin QT_PLUGINS)
    elseif(UNIX)
        install_qt5_plugin(Qt5::QXcbIntegrationPlugin QT_PLUGINS)
        install_qt5_plugin(Qt5::QXcbGlxIntegrationPlugin QT_PLUGINS)
    endif()

    # install to prevent our pyside2 builds import errors this is ugly and should be fixed then we improve our build
    # system
    if(WIN32)
        find_package(
            Qt5
            REQUIRED
            PrintSupport
            UiTools
            Xml
            XmlPatterns
            Help
            Sql
            Svg
            Multimedia
            MultimediaWidgets
            Test
            Qml)
    elseif(APPLE)
        find_package(
            Qt5
            REQUIRED
            PrintSupport
            UiTools
            Xml
            XmlPatterns
            Help
            Sql
            Svg
            Multimedia
            MultimediaWidgets
            Test
            Qml
            DBus
            MacExtras)
    elseif(UNIX)
        find_package(
            Qt5
            REQUIRED
            PrintSupport
            UiTools
            Xml
            XmlPatterns
            Help
            Sql
            Svg
            Multimedia
            MultimediaWidgets
            Test
            Qml
            DBus
            X11Extras)
    endif()

    list(
        APPEND
        QT_DEPS
        Qt5::PrintSupport
        Qt5::Qml
        Qt5::Test
        Qt5::Svg
        Qt5::Help
        Qt5::Sql
        Qt5::Xml
        Qt5::XmlPatterns
        Qt5::Multimedia)
    if(WIN32)
        list(APPEND QT_DEPS Qt5::UiTools)
    elseif(APPLE)
        list(APPEND QT_DEPS Qt5::DBus Qt5::MacExtras Qt5::MultimediaWidgets)
    elseif(UNIX)
        list(APPEND QT_DEPS Qt5::DBus Qt5::X11Extras)
        # for sound playback
        list(APPEND QT_DEPS Qt5::MultimediaWidgets)
    endif()
    foreach(library IN LISTS QT_DEPS)
        get_target_property(shared_lib_path ${library} LOCATION)
        get_filename_component(shared_lib_path ${shared_lib_path} REALPATH)
        list(APPEND INSTALL_SHARED_LIBS ${shared_lib_path})
        if(NOT WIN32 AND NOT APPLE)
            # we need three symlinks but get_filename_component REALPATH, return only full resolve TODO find properly
            # TODO version select
            get_filename_component(library_prefix ${shared_lib_path} NAME_WE)
            get_filename_component(library_dir ${shared_lib_path} DIRECTORY)
            file(GLOB qt_libs "${library_dir}/${library_prefix}.so*")
            list(APPEND INSTALL_SHARED_LIBS ${qt_libs})
        endif()
    endforeach()
    # Qt5::XcbQpa cannot found in our cmake
    if(NOT WIN32 AND NOT APPLE)
        # TODO search properly or at least refactor to cmake function TODO version
        file(GLOB qt_libs "${_qt5Core_install_prefix}/lib/libQt5XcbQpa.so*")
        list(APPEND INSTALL_SHARED_LIBS ${qt_libs})

        get_filename_component(library_prefix ${shared_lib_path} NAME_WE)
        get_filename_component(library_dir ${shared_lib_path} DIRECTORY)
        file(GLOB qt_libs "${library_dir}/${library_prefix}.so*")
        list(APPEND INSTALL_SHARED_LIBS ${qt_libs})

        # for sound playback
        file(GLOB qt_libs "${_qt5Core_install_prefix}/lib/libQt5MultimediaGstTools.so*")
        list(APPEND INSTALL_SHARED_LIBS ${qt_libs})

        get_filename_component(library_prefix ${shared_lib_path} NAME_WE)
        get_filename_component(library_dir ${shared_lib_path} DIRECTORY)
        file(GLOB qt_libs "${library_dir}/${library_prefix}.so*")
        list(APPEND INSTALL_SHARED_LIBS ${qt_libs})
    endif()

    # reported that image plugins is very useful
    install_qt5_plugin(Qt5::QTgaPlugin QT_PLUGINS)
    install_qt5_plugin(Qt5::QGifPlugin QT_PLUGINS)
    install_qt5_plugin(Qt5::QJpegPlugin QT_PLUGINS)
    install_qt5_plugin(Qt5::QTiffPlugin QT_PLUGINS)
    install_qt5_plugin(Qt5::QICOPlugin QT_PLUGINS)
    install_qt5_plugin(Qt5::QSvgPlugin QT_PLUGINS)

    # for sound playback
    if(WIN32)
        install_qt5_plugin(Qt5::WMFServicePlugin QT_PLUGINS)
    elseif(APPLE)
        install_qt5_plugin(Qt5::AVFServicePlugin QT_PLUGINS)
        install_qt5_plugin(Qt5::AVFMediaPlayerServicePlugin QT_PLUGINS)
        install_qt5_plugin(Qt5::CoreAudioPlugin QT_PLUGINS)
    elseif(UNIX)
        install_qt5_plugin(Qt5::QGstreamerAudioDecoderServicePlugin QT_PLUGINS)
        install_qt5_plugin(Qt5::QGstreamerPlayerServicePlugin QT_PLUGINS)
    endif()

    install(
        CODE "file(WRITE \"${CMAKE_INSTALL_PREFIX}/bin/qt.conf\" \"[Paths]\nPrefix=..\nPlugins=qt-plugins\nTranslations = i18n\")"
    )
endif()

if(DCC_INSTALL_PYSIDE2)
    install(DIRECTORY ${SHIBOKEN_PYTHON_MODULE_DIR} DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
    install(DIRECTORY ${PYSIDE_PYTHONPATH} DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
endif()

if(DCC_INSTALL_PYTHON)
    get_filename_component(PYTHON_ROOT_DIR ${PYTHON_INCLUDE_DIR}/../ ABSOLUTE)

    if(WIN32)
        install(
            DIRECTORY ${PYTHON_ROOT_DIR}/DLLs
            DESTINATION python/
            PATTERN ".svn" EXCLUDE)
    elseif(APPLE)
        install(
            DIRECTORY
                /usr/local/Frameworks/Python.framework/Versions/${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/lib-dynload
            DESTINATION lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
    else()
        install(DIRECTORY /usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/lib-dynload
                DESTINATION lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR})
    endif()
    set(PYTHON_EMBED_ARCHIVE_DEST ${CMAKE_INSTALL_PREFIX}/lib)
    if(WIN32)
        set(PYTHON_EMBED_ARCHIVE_DEST ${CMAKE_INSTALL_PREFIX}/bin)
    endif()
    find_file(
        PYTHON_EMBED_ARCHIVE
        NAMES python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.zip
        PATHS ${PYTHON_ROOT_DIR})
    if(PYTHON_EMBED_ARCHIVE)
        install(FILES ${PYTHON_EMBED_ARCHIVE} DESTINATION ${PYTHON_EMBED_ARCHIVE_DEST})
    else()
        install(
            CODE "execute_process(COMMAND \"${PYTHON_EXECUTABLE}\" \"${PROJECT_SOURCE_DIR}/cmake/macros/make_python_stdlib_zip.py\" \"${PYTHON_EMBED_ARCHIVE_DEST}\")"
        )
    endif()
endif()
if(DCC_INSTALL_OCIO AND NOT DCC_HOUDINI_SUPPORT)
    if(NOT DEFINED OCIO_LOCATION)
        set(OCIO_LOCATION /usr/local)
    endif()
    if(WIN32)
        set(OCIO_PYTHON_LIB ${OCIO_LOCATION}/lib/site-packages/PyOpenColorIO.pyd)
    else()
        if(${OCIO_VERSION} VERSION_LESS "2.1" OR APPLE)
            set(OCIO_PYTHON_LIB
                ${OCIO_LOCATION}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/PyOpenColorIO.so
            )
        else()
            set(OCIO_PYTHON_LIB
                ${OCIO_LOCATION}/lib64/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/PyOpenColorIO.so
            )
        endif()
    endif()
    install(FILES ${OCIO_PYTHON_LIB} DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
endif()

if(DCC_INSTALL_OIIO AND NOT DCC_HOUDINI_SUPPORT)
    if(WIN32)
        file(
            GLOB_RECURSE OIIO_PYTHON_LIBS
            "${OIIO_LOCATION}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/OpenImageIO*.pyd")
    elseif(APPLE)
        file(GLOB_RECURSE OIIO_PYTHON_LIBS
             "${OIIO_LOCATION}/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/OpenImageIO*.so")
    else()
        file(
            GLOB_RECURSE OIIO_PYTHON_LIBS
            "${OIIO_LOCATION}/lib64/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/OpenImageIO*.so"
        )
    endif()
    install(FILES ${OIIO_PYTHON_LIBS} DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
endif()

if(DCC_INSTALL_USD)
    if(WIN32)
        install(
            DIRECTORY ${USD_ROOT}/lib/usd
            DESTINATION bin
            USE_SOURCE_PERMISSIONS
            PATTERN "libhdPrman" EXCLUDE)
    else()
        install(
            DIRECTORY ${USD_ROOT}/lib/usd
            DESTINATION lib
            USE_SOURCE_PERMISSIONS
            PATTERN "libhdPrman" EXCLUDE)
    endif()
    install(
        DIRECTORY ${USD_ROOT}/lib/python/pxr
        DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}
        USE_SOURCE_PERMISSIONS
        PATTERN "rmanArgsParser*" EXCLUDE)
    install(
        DIRECTORY ${USD_ROOT}/plugin/usd/
        DESTINATION plugin/usd
        USE_SOURCE_PERMISSIONS
        PATTERN "hdPrmanLoader*" EXCLUDE
        PATTERN "hdxPrman*" EXCLUDE
        PATTERN "hdPrman*" EXCLUDE
        PATTERN "rmanArgsParser*" EXCLUDE
        PATTERN "rmanDiscovery*" EXCLUDE)
endif()

if(DCC_INSTALL_SENTRY)
    get_target_property(SENTRY_CRASHPAD_HANDLER sentry_crashpad::crashpad_handler IMPORTED_LOCATION_RELWITHDEBINFO)
    install(PROGRAMS "${SENTRY_CRASHPAD_HANDLER}" DESTINATION bin)
    if(WIN32)
        get_target_property(SENTRY_CRASHPAD_WER sentry_crashpad::crashpad_wer IMPORTED_LOCATION_RELWITHDEBINFO)
        list(APPEND INSTALL_SHARED_LIBS ${SENTRY_CRASHPAD_WER})
    endif()
endif()

if(DCC_INSTALL_PYTHON_DEPS)
    set(PYTHON_DEPS_ROOT
        ""
        CACHE PATH "PYTHON_DEPS_ROOT")
    if(PYTHON_DEPS_ROOT STREQUAL "")
        message(FATAL_ERROR "PYTHON_DEPS_ROOT is not set")
    endif()
    if(WIN32)
        install(
            DIRECTORY ${PYTHON_DEPS_ROOT}/
            DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}
            PATTERN ".svn" EXCLUDE)
    else()
        install(
            DIRECTORY ${PYTHON_DEPS_ROOT}/
            DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}
            PATTERN ".svn" EXCLUDE
            PATTERN "numpy" EXCLUDE)
        # TODO improve numpy search
        install(DIRECTORY /usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/numpy
                DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
        if(EXISTS /usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/numpy.libs)
            install(
                DIRECTORY /usr/local/lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages/numpy.libs
                DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
        endif()
    endif()
endif()

if(DCC_INSTALL_LUA_DEPS)
    set(LUA_DEPS_ROOT
        ""
        CACHE PATH "LUA_DEPS_ROOT")
    if(LUA_DEPS_ROOT STREQUAL "")
        message(FATAL_ERROR "LUA_DEPS_ROOT is not set")
    endif()
    if(WIN32)
        install(
            DIRECTORY ${LUA_DEPS_ROOT}/
            DESTINATION ${CMAKE_INSTALL_PREFIX}/lua
            PATTERN ".svn" EXCLUDE)
    endif()
endif()

if(DCC_INSTALL_OCIO_DEFAULT_CONFIG)
    set(DCC_OCIO_DEFAULT_CONFIG
        ""
        CACHE
            PATH
            "path to default OCIO config that will be embedded into app, (folder with config.ocio and luts, e.g. nuke-default)"
    )
    if(DCC_OCIO_DEFAULT_CONFIG STREQUAL "")
        message(
            FATAL_ERROR
                "DCC_OCIO_DEFAULT_CONFIG is not set, you must set path to default OCIO config that will be embedded into app"
        )
    endif()
    install(DIRECTORY "${DCC_OCIO_DEFAULT_CONFIG}/" DESTINATION ocio)
endif()

if(DCC_INSTALL_MTLX_DEFAULT_CONFIG)
    set(DCC_MTLX_DEFAULT_CONFIG
        ""
        CACHE PATH "path to default MaterialX config that will be embedded into app, (folder with libraries)")
    if(DCC_MTLX_DEFAULT_CONFIG STREQUAL "")
        message(
            FATAL_ERROR
                "DCC_MTLX_DEFAULT_CONFIG is not set, you must set path to default MaterialX config that will be embedded into app"
        )
    endif()
    install(DIRECTORY "${DCC_MTLX_DEFAULT_CONFIG}/" DESTINATION materialx/libraries)
endif()

if(WIN32)
    install(CODE "file(INSTALL ${INSTALL_SHARED_LIBS} DESTINATION \"${CMAKE_INSTALL_PREFIX}/bin\")")
else()
    install(
        CODE "file(INSTALL ${INSTALL_SHARED_LIBS} DESTINATION \"${CMAKE_INSTALL_PREFIX}/lib\" FOLLOW_SYMLINK_CHAIN)")
endif()

macro(get_all_targets _executables _libraries)
    set(_dirs "")
    set(_queue ${CMAKE_SOURCE_DIR})
    while(_queue)
        list(GET _queue 0 _cur_dir)
        get_directory_property(_subdirs DIRECTORY ${_cur_dir} SUBDIRECTORIES)
        list(APPEND _dirs ${_subdirs})
        list(APPEND _queue ${_subdirs})
        list(REMOVE_AT _queue 0)
    endwhile()

    foreach(_dir ${_dirs})
        if(IS_DIRECTORY ${_dir})
            get_property(
                _targets
                DIRECTORY ${_dir}
                PROPERTY BUILDSYSTEM_TARGETS)
            foreach(_target ${_targets})
                get_target_property(_target_type ${_target} TYPE)
                if(_target_type STREQUAL "EXECUTABLE")
                    list(APPEND _executables $<TARGET_FILE:${_target}>)
                elseif(_target_type STREQUAL "SHARED_LIBRARY")
                    list(APPEND _libraries $<TARGET_FILE:${_target}>)
                endif()
            endforeach()
        endif()
    endforeach()
endmacro()

get_all_targets(_executables _libraries)

if(WIN32)
    if(DCC_INSTALL_ADS)
        get_target_property(_ads_lib ads::qtadvanceddocking LOCATION)
        get_filename_component(_ads_lib_dir ${_ads_lib} DIRECTORY)
        list(APPEND _search_dirs ${_ads_lib_dir})
    endif()
    if(DCC_INSTALL_ALEMBIC)
        get_filename_component(_alembic_library_dir ${ALEMBIC_LIBRARY} DIRECTORY)
        list(APPEND _search_dirs ${_alembic_library_dir})
    endif()
    if(DCC_INSTALL_BOOST)
        list(APPEND _search_dirs ${Boost_LIBRARY_DIR_RELEASE})
    endif()
    if(DCC_INSTALL_EMBREE)
        list(APPEND _search_dirs ${EMBREE_LOCATION}/bin)
    endif()
    if(DCC_BUILD_EDITORIAL)
        get_target_property(_otio_lib OTIO::opentimelineio LOCATION)
        get_filename_component(_otio_lib_dir ${_otio_lib} DIRECTORY)
        list(APPEND _search_dirs ${_otio_lib_dir})
    endif()
    if(DCC_INSTALL_GLEW)
        list(APPEND _search_dirs ${GLEW_LOCATION}/bin)
    endif()
    if(DCC_INSTALL_ILMBASE)
        list(APPEND _search_dirs ${ILMBASE_HOME}/bin)
    endif()
    if(DCC_INSTALL_OCIO)
        list(APPEND _search_dirs ${OCIO_LOCATION}/bin)
    endif()
    if(DCC_INSTALL_OIIO)
        list(APPEND _search_dirs ${OIIO_LOCATION}/bin)
    endif()
    if(DCC_INSTALL_OPENSUBDIV)
        list(APPEND _search_dirs ${OPENSUBDIV_ROOT}/bin) # in case of dynamic subdiv
    endif()
    if(DCC_INSTALL_OSL)
        list(APPEND _search_dirs ${OSL_ROOT}/bin)
    endif()
    if(DCC_INSTALL_PARTIO)
        get_filename_component(_partio_lib_dir ${PARTIO_LIBRARY} DIRECTORY) # todo: move it to Findpartio.cmake later
        list(APPEND _search_dirs ${_partio_lib_dir}/../bin)
    endif()
    if(DEFINED PTEX_LIBRARY AND DCC_INSTALL_PTEX)
        get_filename_component(_ptex_lib_dir ${PTEX_LIBRARY} DIRECTORY) # todo: move it to Findptex.cmake later
        list(APPEND _search_dirs ${_ptex_lib_dir})
    endif()
    if(DCC_INSTALL_PYSIDE2)
        get_target_property(_pyside_lib PySide2::pyside2 LOCATION)
        get_filename_component(_pyside_lib_dir ${_pyside_lib} DIRECTORY)
        get_target_property(_shiboken_lib Shiboken2::libshiboken LOCATION)
        get_filename_component(_shiboken_lib_dir ${_shiboken_lib} DIRECTORY)
        list(APPEND _search_dirs ${_shiboken_lib_dir})
    endif()
    if(DCC_INSTALL_PYTHON)
        list(APPEND _search_dirs ${PYTHON_INCLUDE_DIR}/..)
        # Install python dll manually because python27.dll might be located in system32 which is excluded during
        # dependency search
        if(WIN32)
            get_filename_component(_python_root ${PYTHON_EXECUTABLE} DIRECTORY)
            get_filename_component(_python_lib_name ${PYTHON_LIBRARY} NAME_WE)
            file(INSTALL "${_python_root}/${_python_lib_name}.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
            if(DCC_USE_PYTHON_3)
                file(INSTALL "${_python_root}/python3.dll" DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
            endif()
        endif()
    endif()
    if(DCC_INSTALL_QT5)
        get_target_property(_qt_lib_path Qt5::Core LOCATION)
        get_filename_component(_qt_lib_dir ${_qt_lib_path} DIRECTORY)
        list(APPEND _search_dirs ${_qt_lib_dir})
    endif()
    if(DCC_INSTALL_TBB)
        list(APPEND _search_dirs ${TBB_ROOT_DIR}/bin)
    endif()
    if(DCC_INSTALL_USD)
        list(APPEND _search_dirs ${USD_LIBRARY_DIR})
    endif()
    if(DCC_INSTALL_ZMQ)
        list(APPEND _search_dirs "${ZMQ_ROOT}/bin")
    endif()
    if(DCC_INSTALL_ARNOLD AND DCC_BUILD_ARNOLD_SUPPORT)
        list(APPEND _search_dirs "${USDARNOLD_LIBRARY_DIR}" "${ARNOLDUSD_LIBRARY_DIR}" "${ARNOLD_ROOT}/bin")
    endif()
    if(DCC_INSTALL_LUA)
        get_filename_component(_lua_library_dir ${LUA_LIBRARY} DIRECTORY)
        get_filename_component(_lua_root_dir ${_lua_library_dir} DIRECTORY)
        list(APPEND _search_dirs "${_lua_root_dir}/bin")
    endif()
    if(DEFINED TURBO_ACTIVATE_ROOT)
        list(APPEND _search_dirs ${TURBO_ACTIVATE_ROOT}/bin)
    endif()
    if(DEFINED WIN_SPARKLE_ROOT)
        list(APPEND _search_dirs ${WIN_SPARKLE_ROOT}/bin)
    endif()
    if(DCC_INSTALL_OPENVDB)
        list(APPEND _search_dirs ${OPENVDB_ROOT}/bin)
    endif()
else()
    get_target_property(_pyside_lib PySide2::pyside2 LOCATION)
    get_filename_component(_pyside_lib_dir ${_pyside_lib} DIRECTORY)
    get_target_property(_shiboken_lib Shiboken2::libshiboken LOCATION)
    get_filename_component(_shiboken_lib_dir ${_shiboken_lib} DIRECTORY)
    set(_search_dirs "${SHIBOKEN_CLANG_INSTALL_DIR}" "${ARNOLD_ROOT}/bin")
    if(DCC_HOUDINI_SUPPORT)
        list(APPEND _search_dirs "${HOUDINI_ROOT}/dsolib")
    endif()
    if(DCC_KATANA_SUPPORT)
        list(APPEND _search_dirs "${KATANA_ROOT}/bin")
    endif()
endif()

install(
    CODE "
	set(_executable_targets \"${_executables}\")
	set(_library_targets \"${_libraries}\")
	set(_search_dirs \"${_search_dirs}\")
	set(_houdini_support ${DCC_HOUDINI_SUPPORT})
	set(_katana_support ${DCC_KATANA_SUPPORT})

    set(_excluded_install_targets )
")
install(
    CODE [=[
	if (WIN32)
		file(GET_RUNTIME_DEPENDENCIES
									EXECUTABLES ${_executable_targets}
									LIBRARIES ${_library_targets}
									RESOLVED_DEPENDENCIES_VAR _resolved
									UNRESOLVED_DEPENDENCIES_VAR _unresolved
									POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" "msvcp" "vcruntime" ${_excluded_install_targets}
									DIRECTORIES ${_search_dirs})

		## CMake's file(GET_RUNTIME_DEPENDENCIES) converts all window paths to lowercase
		## We'd like to preserve original file name
		foreach(dependency ${_resolved})
			file(GLOB _case_sensitive_dep ${dependency})
			list(APPEND _case_sensitive_deps ${_case_sensitive_dep})
		endforeach()

		file(INSTALL ${_case_sensitive_deps}
			DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
	elseif(UNIX)
		SET (_excluded_libs
			"ld-linux-x86-64.so"
			"libstdc\\+\\+.so"
			"librt.so"
			"libpthread.so"
			"libsystemd.so"
			"libgpg-error.so"
			"libm.so"
			"libgcc_s.so"
			"libdl.so"
			"libc.so"
			"libxcb.so"
			"libXext.so"
			"libXau.so"
			"libX11.so"
			"libGLX.so"
			"libGLU.so"
			"libGL.so"
			"libGLdispatch.so"
			"libSM.so"
			"libICE.so"
			"libgthread.so"
			"libglib.so"
			"libuuid.so"
			"libutil.so"
			"libOpenGL.so"
			"libai.so"
			"libusdArnold.so"
			"ndrArnold.so"
			"resolv"
			"selinux"
			"${_library_targets}"
            "${_excluded_install_targets}")

			if (NOT APPLE)
				file(STRINGS /etc/os-release distro_name_string REGEX [[^ID=.*$]])
				string(REGEX MATCHALL [["\\?(.*)"\\?]] _ ${distro_name_string})
				set(distro_id ${CMAKE_MATCH_1})
				if (distro_id EQUAL "astra")
					list(APPEND _excluded_libs "tinfo")
				endif()
			endif()


		if (_houdini_support)
			list(APPEND _excluded_libs
				"/usr/local/lib/libpython"
				"/usr/local/lib/libtbb"
				"${HOUDINI_ROOT}/dsolib"
				"${HOUDINI_ROOT}/python/lib")
		endif()
		if (_katana_support)
			list(APPEND _excluded_libs
				"${KATANA_ROOT}/bin")
		endif()
		file(GET_RUNTIME_DEPENDENCIES
			EXECUTABLES ${_executable_targets}
			LIBRARIES ${_library_targets}
      UNRESOLVED_DEPENDENCIES_VAR _unresolved
	  RESOLVED_DEPENDENCIES_VAR _resolved
			POST_EXCLUDE_REGEXES ${_excluded_libs} ${_excluded_install_targets}
			DIRECTORIES ${_search_dirs}
		)
		file(INSTALL ${_resolved}
			DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
			FOLLOW_SYMLINK_CHAIN
			TYPE SHARED_LIBRARY)
	endif()
]=])

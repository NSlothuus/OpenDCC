set(PLUGIN_DIR ${CMAKE_INSTALL_PREFIX}/packages/opendcc.usd_editor.live_share/)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
    DESTINATION "${PLUGIN_DIR}"
    FILES_MATCHING
    PATTERN "*.py")
if(PYTHON_VERSION_MAJOR VERSION_LESS 3)
    install(
        CODE "file(WRITE \"${PLUGIN_DIR}/python/opendcc/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
    install(
        CODE "file(WRITE \"${PLUGIN_DIR}/python/opendcc/usd_editor/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
endif()
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/package.toml DESTINATION ${PLUGIN_DIR})

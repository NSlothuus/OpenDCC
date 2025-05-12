install(DIRECTORY opendcc DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT})
# python2 old namespace packages
if(PYTHON_VERSION_MAJOR VERSION_LESS 3)
    install(
        CODE "file(WRITE \"${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc/__init__.py\" \"try:\n    __import__(\"pkg_resources\").declare_namespace(__name__)\nexcept:\n    from pkgutil import extend_path\n    __path__ = extend_path(__path__, __name__)\")"
    )
endif()

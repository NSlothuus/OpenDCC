if(WIN32)
    set(_runtime_dir bin)
else()
    set(_runtime_dir plugin/usd)
endif()

opendcc_make_usd_schema(
    usdUIExt
    AFTER_GEN_SCRIPT
    fix_generated.py
    ${USD_VERSION}
    VS_FOLDER
    usdUIExt
    PLUGIN_DIR
    usd
    SCHEMA_FILE
    schema.usda
    PYMODULE_CPPFILES
    module.cpp
    PYTHON_FILES
    __init__.py
    HEADER_INSTALL_DIR
    "include/opendcc/usd/usdUIExt"
    SRC_GENERATION_DIR
    "usdUIExt"
    CUSTOM_RUNTIME_DIR
    ${_runtime_dir})

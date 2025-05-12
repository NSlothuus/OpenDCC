add_definitions(-DNOMINMAX)

set(PY_USD_FALLBACK_PROXY_SRC main.cpp)

add_library(py_usd_fallback_proxy SHARED ${PY_USD_FALLBACK_PROXY_SRC})

target_compile_definitions(py_usd_fallback_proxy PRIVATE MODULE_NAME=usd_fallback_proxy)

target_include_directories(
    py_usd_fallback_proxy PUBLIC ${PROJECT_SOURCE_DIR}/src/lib/ ${USD_FALLBACK_PROXY_INCLUDE_DIR}
                                 ${PYTHON_INCLUDE_DIRS} ${USD_INCLUDE_DIR})

target_link_libraries(py_usd_fallback_proxy py_utils usd_fallback_proxy ${PYTHON_LIBRARIES} pybind11::pybind11)

if(MSVC)
    set_target_properties(
        py_usd_fallback_proxy
        PROPERTIES PREFIX ""
                   OUTPUT_NAME usd_fallback_proxy
                   CLEAN_DIRECT_OUTPUT 1
                   SUFFIX ".pyd")
else()
    set_target_properties(
        py_usd_fallback_proxy
        PROPERTIES PREFIX ""
                   OUTPUT_NAME usd_fallback_proxy
                   CLEAN_DIRECT_OUTPUT 1
                   SUFFIX ".so")
endif()

install(
    TARGETS py_usd_fallback_proxy
    RUNTIME DESTINATION "${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc"
    LIBRARY DESTINATION "${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc")

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:py_usd_fallback_proxy>
        DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc
        OPTIONAL)
endif()

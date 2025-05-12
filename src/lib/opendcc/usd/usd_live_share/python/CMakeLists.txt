set(TARGET_NAME usd_live_share)

add_library(py_${TARGET_NAME} SHARED wrap_live_share_session.cpp)
target_compile_definitions(
    py_${TARGET_NAME}
    PUBLIC -DBOOST_ALL_DYN_LINK
    PRIVATE MODULE_NAME=usd_live_share)

target_link_libraries(py_${TARGET_NAME} ${PYTHON_LIBRARIES} pybind11::pybind11 ${TARGET_NAME})

if(MSVC)
    set_target_properties(
        py_${TARGET_NAME}
        PROPERTIES PREFIX ""
                   OUTPUT_NAME ${TARGET_NAME}
                   CLEAN_DIRECT_OUTPUT 1
                   SUFFIX ".pyd")
else()
    set_target_properties(
        py_${TARGET_NAME}
        PROPERTIES PREFIX ""
                   OUTPUT_NAME ${TARGET_NAME}
                   CLEAN_DIRECT_OUTPUT 1
                   SUFFIX ".so")
endif()
if(DCC_PACKAGE_OPENDCC_USD_EDITOR_LIVE_SHARE)
    install(
        TARGETS py_${TARGET_NAME}
        RUNTIME DESTINATION "packages/opendcc.usd_editor.live_share/python/opendcc/usd_editor/live_share"
        LIBRARY DESTINATION "packages/opendcc.usd_editor.live_share/python/opendcc/usd_editor/live_share")

    if(MSVC)
        install(
            FILES $<TARGET_PDB_FILE:py_${TARGET_NAME}>
            DESTINATION packages/opendcc.usd_editor.live_share/python/opendcc/usd_editor/live_share
            OPTIONAL)
    endif()
endif()

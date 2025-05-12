add_library(color_theme_python SHARED wrap_color_theme.cpp)

make_folder("opendcc/ui" color_theme_python)

target_link_libraries(color_theme_python color_theme pybind11::pybind11 ${PYTHON_LIBRARY})

if(MSVC)
    set_target_properties(color_theme_python PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(color_theme_python PROPERTIES SUFFIX ".so")
endif()
set_target_properties(color_theme_python PROPERTIES PREFIX "" OUTPUT_NAME color_theme)
install(
    TARGETS color_theme_python
    RUNTIME DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc
    LIBRARY DESTINATION ${DCC_PYTHON_INSTALL_SITE_PACKAGES_ROOT}/opendcc)

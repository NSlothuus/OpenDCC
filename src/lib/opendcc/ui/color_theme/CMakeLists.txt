add_library(color_theme SHARED api.h color_theme.h color_theme.cpp)

target_compile_definitions(color_theme PRIVATE -DOPENDCC_COLOR_THEME_EXPORT)

make_folder("opendcc/ui" color_theme)

install(
    TARGETS color_theme
    EXPORT color_theme-targets
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib"
    RUNTIME DESTINATION "bin")
install(
    EXPORT color_theme-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/color_theme
    NAMESPACE opendcc::)

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:color_theme>
        DESTINATION bin
        OPTIONAL)
endif()
add_subdirectory(wrap_color_theme)

set(RENDER_VIEW_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(thirdparty/qtsingleapplication)
add_subdirectory(src)

make_folder("render_view" render_view qtsingleapplication)
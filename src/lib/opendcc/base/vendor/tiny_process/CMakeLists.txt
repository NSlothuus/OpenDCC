find_package(Threads REQUIRED)

set(tiny_process_h process.hpp)
if(WIN32)
    set(tiny_process_cpp process.cpp process_win.cpp)
else()
    set(tiny_process_cpp process.cpp process_unix.cpp)
endif()

add_library(opendcc_base_tiny_process STATIC ${tiny_process_h} ${tiny_process_cpp})
make_folder("base/vendor" opendcc_base_tiny_process)

target_link_libraries(opendcc_base_tiny_process ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(opendcc_base_tiny_process PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES ${tiny_process_h} DESTINATION "include/opendcc/base/tiny_process/")
install(
    TARGETS opendcc_base_tiny_process
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

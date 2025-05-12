set(TARGET_NAME test_runner)

add_library(${TARGET_NAME} SHARED main.cpp)

make_folder("base" ${TARGET_NAME})

target_link_libraries(${TARGET_NAME} doctest::doctest)
install(TARGETS ${TARGET_NAME} EXPORT test_runner-targets)
install(
    EXPORT test_runner-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/test_runner
    NAMESPACE opendcc::)
if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:${TARGET_NAME}>
        DESTINATION bin
        OPTIONAL)
endif()

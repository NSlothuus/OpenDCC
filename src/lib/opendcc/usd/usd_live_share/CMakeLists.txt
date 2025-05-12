set(TARGET_NAME usd_live_share)

add_library(
    ${TARGET_NAME} SHARED
    api.h
    live_share_edits.h
    live_share_edits.cpp
    live_share_state_delegate.h
    live_share_state_delegate.cpp
    live_share_session.h
    live_share_session.cpp)

target_compile_definitions(${TARGET_NAME} PRIVATE -DUSD_LIVE_SHARE_EXPORT -DBOOST_ALL_DYN_LINK)

target_link_libraries(${TARGET_NAME} ${Boost_SYSTEM_LIBRARY} layer_tree_watcher usd_ipc_serialization utils
                      Threads::Threads)
if(CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(${TARGET_NAME} ${CMAKE_THREAD_LIBS_INIT})
endif()

install(
    TARGETS ${TARGET_NAME}
    EXPORT usd_live_share-targets
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    LIBRARY DESTINATION "lib")

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:${TARGET_NAME}>
        DESTINATION bin
        OPTIONAL)
endif()

install(
    EXPORT usd_live_share-targets
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/${TARGET_NAME}
    NAMESPACE opendcc::)

add_subdirectory(python)

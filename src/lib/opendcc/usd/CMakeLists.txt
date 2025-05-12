add_subdirectory(hydra_render_session_api)
add_subdirectory(layer_tree_watcher)

add_subdirectory(usd_ipc_serialization)
add_subdirectory(usd_live_share)
if(DCC_BUILD_RENDER_VIEW)
    add_subdirectory(render)
endif()

if(USD_VERSION VERSION_GREATER_EQUAL 0.22.05)
    add_subdirectory(compositing)
endif()

make_folder(
    "usd/dcc"
    hydra_render_session_api
    hydra_render_session_api_py
    layer_tree_watcher
    usd_ipc_serialization
    usd_live_share
    py_usd_live_share)

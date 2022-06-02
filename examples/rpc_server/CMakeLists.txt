add_executable_m7(rpc_server
    rpc_server.cc
)

target_link_libraries(rpc_server
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
)

add_executable_m7(RPCServer
    rpc_server.cc
)

target_link_libraries(RPCServer
    libs_base-m7_freertos
    libs_rpc_http_server
    libs_mjson
)

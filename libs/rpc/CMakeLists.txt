add_library_m7(libs_rpc_http_server STATIC
    rpc_http_server.cc
)

target_link_libraries(libs_rpc_http_server
    libs_mjson
    libs_base-m7_freertos
    libs_base-m7_http_server
)

cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_RPCServerIOHTTP STATIC
    ${VALIANT_SOURCE_DIR}/libs/RPCServer/rpc_server_io_http.cc
)

target_link_libraries(libs_RPCServerIOHTTP
    libs_mjson
    libs_base-m7_freertos
)

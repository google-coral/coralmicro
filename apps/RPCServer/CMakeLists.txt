cmake_minimum_required(VERSION 3.13)
project(RPCServer)

include_directories(.)

add_executable_m7(RPCServer
    rpc_server.cc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/lwip/src/apps/httpsrv/httpsrv_base64.c
)

target_link_libraries(RPCServer
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_mjson
)

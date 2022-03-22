cmake_minimum_required(VERSION 3.13)
project(RPCServer)

include_directories(.)

add_executable_m7(RPCServer
    rpc_server.cc
)

target_link_libraries(RPCServer
    libs_base-m7_freertos
    libs_RPCServerIOHTTP
    libs_mjson
)

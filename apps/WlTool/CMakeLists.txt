cmake_minimum_required(VERSION 3.13)
project(WlTool)

include_directories(.)

add_executable_m7(WlTool
    wl_tool.cc
)

target_include_directories(WlTool PRIVATE
    .
)

target_link_libraries(WlTool
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_testlib
    libs_nxp_rt1176-sdk_wiced_mfgtest
)

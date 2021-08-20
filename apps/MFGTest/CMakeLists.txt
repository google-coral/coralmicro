cmake_minimum_required(VERSION 3.13)
project(MFGTest)

include_directories(.)

add_executable_m7(MFGTest
    mfg_test.cc
)

target_link_libraries(MFGTest
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_mjson
    libs_testconv1
    libs_tpu_freertos
)

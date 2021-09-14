cmake_minimum_required(VERSION 3.13)
project(RackTest)

include_directories(.)

add_executable_m7(RackTest
    rack_test.cc
)

target_link_libraries(RackTest
    libs_base-m7_freertos
    libs_posenet
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_tensorflow-m7
    libs_testlib
    libs_tpu_freertos
    libs_mjson
)

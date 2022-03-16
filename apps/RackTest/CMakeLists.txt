cmake_minimum_required(VERSION 3.13)
project(RackTest)

include_directories(.)

add_executable_m7(RackTest
    rack_test.cc
    M4_EXECUTABLE
    RackTestM4
)

target_link_libraries(RackTest
    libs_base-m7_freertos
    libs_coremark-m7
    libs_mjson
    libs_posenet
    libs_RPCServerIOHTTP
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_tensorflow-m7
    libs_testlib
    libs_tpu_freertos
)

add_executable_m4(RackTestM4
    rack_test_m4.cc
)

target_link_libraries(RackTestM4
    libs_base-m4_freertos
    libs_coremark-m4
)

set(RACK_TEST_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_coremark-m7
    libs_mjson
    libs_posenet
    libs_rpc_http_server
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_tensorflow-m7
    libs_testlib
    libs_tpu_freertos
)

add_executable_m7(RackTest
    rack_test.cc
    M4_EXECUTABLE
    RackTestM4
)

target_link_libraries(RackTest
    ${RACK_TEST_LINK_LIBRARIES}
)

add_executable_m4(RackTestM4
    rack_test_m4.cc
)

target_link_libraries(RackTestM4
    libs_base-m4_freertos
    libs_coremark-m4
)


# For wifi tests.
add_executable_m7(RackTestWifi
    rack_test.cc
    M4_EXECUTABLE
    RackTestM4
)

target_compile_definitions(RackTestWifi PUBLIC TEST_WIFI)

target_link_libraries(RackTestWifi
    ${RACK_TEST_LINK_LIBRARIES}
    libs_base-m7_wifi
)
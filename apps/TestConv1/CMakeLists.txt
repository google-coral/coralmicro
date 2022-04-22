add_executable_m7(TestConv1
    testconv1.cc
)

target_link_libraries(TestConv1
    libs_base-m7_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_testconv1
    libs_tpu_freertos
)

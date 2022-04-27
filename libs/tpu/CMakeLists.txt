add_library_m7(libs_tpu_freertos STATIC
    edgetpu_executable.cc
    edgetpu_manager.cc
    edgetpu_op.cc
    tpu_driver.cc
)

target_link_libraries(libs_tpu_freertos
    libs_base-m7_freertos
    libs_usb_host_edgetpu_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tensorflow-m7
    libs_FreeRTOS
    libs_flatbuffers
)

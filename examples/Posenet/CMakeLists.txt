add_executable_m7(Posenet
    posenet.cc
)

target_link_libraries(Posenet
    libs_base-m7_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_posenet
    libs_tensorflow-m7
    libs_tpu_freertos
)

add_executable_m7(YAMNet
    yamnet.cc
)

target_link_libraries(YAMNet
    libs_base-m7_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_yamnet
    libs_tensorflow-m7
    libs_tpu_freertos
)

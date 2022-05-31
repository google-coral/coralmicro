add_executable_m7(YAMNet
    yamnet.cc

    DATA
    ${CMAKE_SOURCE_DIR}/models/yamnet_test_audio.bin
)

target_link_libraries(YAMNet
    libs_base-m7_freertos
    libs_tasks_AudioTask_freertos
    libs_yamnet
)

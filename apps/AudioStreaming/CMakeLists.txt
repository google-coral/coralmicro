add_executable_m7(AudioStreaming
    audio_server.cc
    network.cc
)

target_link_libraries(AudioStreaming
    libs_base-m7_freertos
    libs_tasks_AudioTask_freertos
)

add_executable_m7(audio_streaming
    audio_server.cc
)

target_link_libraries(audio_streaming
    libs_base-m7_freertos
    libs_base-m7_network
    libs_audio_freertos
)

cmake_minimum_required(VERSION 3.13)
project(AudioStreaming)

include_directories(.)

add_executable_m7(AudioStreaming
    audio_server.cc
)

target_include_directories(AudioStreaming PRIVATE
    .
)

target_link_libraries(AudioStreaming
    libs_base-m7_freertos
    libs_tasks_AudioTask_freertos
)

cmake_minimum_required(VERSION 3.13)
project(MessagePassing)

include_directories(.)

add_executable_m7(MessagePassing
    message_passing.cc
    M4_EXECUTABLE
    MessagePassingM4
)

target_link_libraries(MessagePassing
    libs_base-m7_freertos
)

add_executable_m4(MessagePassingM4
    message_passing_m4.cc
)

target_link_libraries(MessagePassingM4
    libs_base-m4_freertos
)
add_executable_m7(message_passing
    message_passing.cc
    M4_EXECUTABLE
    message_passing_m4
)

target_link_libraries(message_passing
    libs_base-m7_freertos
)

add_executable_m4(message_passing_m4
    message_passing_m4.cc
)

target_link_libraries(message_passing_m4
    libs_base-m4_freertos
)

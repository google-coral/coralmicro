add_executable_m7(socket_hello_world
    socket_hello_world.cc
)

target_link_libraries(socket_hello_world
    libs_base-m7_freertos
)

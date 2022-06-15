add_executable_m7(freertos_hello_world
    freertos_hello_world.cc
)

target_link_libraries(freertos_hello_world
    libs_base-m7_freertos
)

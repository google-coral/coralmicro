add_executable_m7(freertos_multi_core
    hello_world_cm7.cc
    M4_EXECUTABLE
    freertos_multi_core_m4
)

target_link_libraries(freertos_multi_core
    libs_base-m7_freertos
)

add_executable_m4(freertos_multi_core_m4
    hello_world_cm4.cc
)

target_link_libraries(freertos_multi_core_m4
    libs_base-m4_freertos
)

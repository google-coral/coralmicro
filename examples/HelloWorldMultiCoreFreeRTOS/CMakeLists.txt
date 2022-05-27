add_executable_m7(HelloWorldMultiCoreFreeRTOS
    hello_world_cm7.cc
    M4_EXECUTABLE
    HelloWorldMultiCoreFreeRTOSM4
)

target_link_libraries(HelloWorldMultiCoreFreeRTOS
    libs_base-m7_freertos
)

add_executable_m4(HelloWorldMultiCoreFreeRTOSM4
    hello_world_cm4.cc
)

target_link_libraries(HelloWorldMultiCoreFreeRTOSM4
    libs_base-m4_freertos
)

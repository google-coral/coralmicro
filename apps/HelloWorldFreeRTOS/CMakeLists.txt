cmake_minimum_required(VERSION 3.13)
project(HelloWorldFreeRTOS)

include_directories(.)

add_executable(HelloWorldFreeRTOS
    hello_world_freertos.cc
    board.c
    clock_config.c
    peripherals.c
    pin_mux.c
)

target_link_libraries(HelloWorldFreeRTOS
    libs_FreeRTOS
    libs_nxp_rt1176-sdk
)

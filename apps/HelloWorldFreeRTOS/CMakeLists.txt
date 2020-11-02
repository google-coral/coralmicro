cmake_minimum_required(VERSION 3.13)
project(HelloWorldFreeRTOS)

include_directories(.)

add_executable(HelloWorldFreeRTOS
    board.c
    clock_config.c
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    hello_world_freertos.cc
    peripherals.c
    pin_mux.c
)

target_include_directories(HelloWorldFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloWorldFreeRTOS
    libs_nxp_rt1176-sdk_freertos
)

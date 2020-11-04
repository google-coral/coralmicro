cmake_minimum_required(VERSION 3.13)
project(HelloTensorflowFreeRTOS)

include_directories(.)

add_executable(HelloTensorflowFreeRTOS
    board.c
    clock_config.c
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    hello_tensorflow_freertos.cc
    peripherals.c
    pin_mux.c
)

target_include_directories(HelloTensorflowFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloTensorflowFreeRTOS
    libs_nxp_rt1176-sdk_freertos
    libs_tensorflow
    libs_tensorflow_model_hello_world
)

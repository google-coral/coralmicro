cmake_minimum_required(VERSION 3.13)
project(HelloTensorflow)

add_executable(HelloTensorflow
    board.c
    clock_config.c
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    hello_tensorflow.cc
    peripherals.c
    pin_mux.c
)

target_include_directories(HelloTensorflow PRIVATE
    .
)

target_link_libraries(HelloTensorflow
    libs_nxp_rt1176-sdk_bm
    libs_tensorflow
    libs_tensorflow_model_hello_world
)

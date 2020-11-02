cmake_minimum_required(VERSION 3.13)
project(HelloTensorflow)

add_executable(HelloTensorflow
    hello_tensorflow.cc
    board.c
    clock_config.c
    peripherals.c
    pin_mux.c
)

target_link_libraries(HelloTensorflow
    libs_nxp_rt1176-sdk
    libs_tensorflow
    libs_tensorflow_model_hello_world
)

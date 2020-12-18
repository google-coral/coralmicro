cmake_minimum_required(VERSION 3.13)
project(HelloTensorflow)

add_executable_m7(HelloTensorflow
    hello_tensorflow.cc
)

target_include_directories(HelloTensorflow PRIVATE
    .
)

target_link_libraries(HelloTensorflow
    libs_nxp_rt1176-sdk_bm
    libs_tensorflow
    libs_tensorflow_model_hello_world
)
target_sources(HelloTensorflow PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)

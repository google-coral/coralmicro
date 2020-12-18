cmake_minimum_required(VERSION 3.13)
project(HelloTensorflowFreeRTOS)

include_directories(.)

add_executable_m7(HelloTensorflowFreeRTOS
    hello_tensorflow_freertos.cc
)

target_include_directories(HelloTensorflowFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloTensorflowFreeRTOS
    libs_nxp_rt1176-sdk_freertos
    libs_tensorflow
    libs_tensorflow_model_hello_world
)
target_sources(HelloTensorflowFreeRTOS PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)

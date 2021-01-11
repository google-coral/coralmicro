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
    libs_base-m7_freertos
    libs_tensorflow-m7
    libs_tensorflow_model_hello_world
)

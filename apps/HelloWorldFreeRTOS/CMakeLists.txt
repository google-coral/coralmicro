cmake_minimum_required(VERSION 3.13)
project(HelloWorldFreeRTOS)

include_directories(.)

add_executable_m7(HelloWorldFreeRTOS
    hello_world_freertos.cc
)

target_include_directories(HelloWorldFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloWorldFreeRTOS
    libs_base-m7_freertos
)

cmake_minimum_required(VERSION 3.13)
project(HelloWorldFreeRTOS)

include_directories(.)

add_executable(HelloWorldFreeRTOS
    hello_world_freertos.cc
)

target_include_directories(HelloWorldFreeRTOS PRIVATE
    .
)

target_link_libraries(HelloWorldFreeRTOS
    libs_nxp_rt1176-sdk_freertos
)
target_sources(HelloWorldFreeRTOS PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_freertos>)

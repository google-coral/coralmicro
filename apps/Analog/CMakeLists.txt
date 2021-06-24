cmake_minimum_required(VERSION 3.13)
project(Analog)

include_directories(.)

add_executable_m7(Analog
    analog.cc
)

target_include_directories(Analog PRIVATE
    .
)

target_link_libraries(Analog
    libs_base-m7_freertos
)

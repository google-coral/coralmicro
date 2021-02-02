cmake_minimum_required(VERSION 3.13)
project(DMATest)

include_directories(.)

add_executable_m7(DMATest
    dma_test.cc
)

target_include_directories(DMATest PRIVATE
    .
)

target_link_libraries(DMATest
    libs_base-m7_freertos
)

cmake_minimum_required(VERSION 3.13)
project(SDRAMTest)

include_directories(.)

add_executable_m7(SDRAMTest
    sdram_test.cc
)

target_include_directories(SDRAMTest PRIVATE
    .
)

target_link_libraries(SDRAMTest
    libs_base-m7_freertos
)

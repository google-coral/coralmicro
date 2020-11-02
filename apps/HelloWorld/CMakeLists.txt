cmake_minimum_required(VERSION 3.13)
project(HelloWorld)

add_executable(HelloWorld
    board.c
    clock_config.c
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    hello_world.cc
    peripherals.c
    pin_mux.c
)

target_include_directories(HelloWorld PRIVATE
    .
)

target_link_libraries(HelloWorld
    libs_nxp_rt1176-sdk_bm
)

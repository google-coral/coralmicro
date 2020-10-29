cmake_minimum_required(VERSION 3.13)
project(HelloWorld)

add_executable(HelloWorld
    hello_world.cc
    board.c
    clock_config.c
    peripherals.c
    pin_mux.c
)

target_link_libraries(HelloWorld
    libs_nxp_rt1176-sdk
)

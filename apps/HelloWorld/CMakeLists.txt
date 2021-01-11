cmake_minimum_required(VERSION 3.13)
project(HelloWorld)

add_executable_m7(HelloWorld
    hello_world.cc
)

target_include_directories(HelloWorld PRIVATE
    .
)

target_link_libraries(HelloWorld
    libs_base-m7_bm
)

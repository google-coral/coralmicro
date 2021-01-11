cmake_minimum_required(VERSION 3.13)
project(HelloWorldMultiCore)

add_executable_m7(HelloWorldMultiCore
    hello_world_cm7.cc
    HelloWorldMultiCoreM4.obj
)

target_include_directories(HelloWorldMultiCore PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCore
    libs_base-m7_bm
)

add_executable_m4(HelloWorldMultiCoreM4
    hello_world_cm4.cc
)
target_include_directories(HelloWorldMultiCoreM4 PRIVATE
    .
)

target_link_libraries(HelloWorldMultiCoreM4
    libs_base-m4_bm
)

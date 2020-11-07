cmake_minimum_required(VERSION 3.13)
project(HelloWorld)

add_executable(HelloWorld
    hello_world.cc
)

target_include_directories(HelloWorld PRIVATE
    .
)

target_link_libraries(HelloWorld
    libs_nxp_rt1176-sdk_bm
)
target_sources(HelloWorld PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)

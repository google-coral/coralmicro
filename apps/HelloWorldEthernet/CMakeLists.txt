cmake_minimum_required(VERSION 3.13)
project(HelloWorldEthernet)

include_directories(.)

add_executable_m7(HelloWorldEthernet
    hello_world_ethernet.cc
)

target_include_directories(HelloWorldEthernet PRIVATE
    .
)

target_link_libraries(HelloWorldEthernet
    libs_base-m7_freertos
    libs_base-m7_ethernet
    libs_curl
)

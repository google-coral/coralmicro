cmake_minimum_required(VERSION 3.13)
project(HelloSocket)

include_directories(.)

add_executable_m7(HelloSocket
    hello_socket.cc
)

target_include_directories(HelloSocket PRIVATE
    .
)

target_link_libraries(HelloSocket
    libs_base-m7_freertos
)

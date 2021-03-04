cmake_minimum_required(VERSION 3.13)
project(CameraTest)

include_directories(.)

add_executable_m7(CameraTest
    camera_test.cc
)

target_include_directories(CameraTest PRIVATE
    .
)

target_link_libraries(CameraTest
    libs_base-m7_freertos
)

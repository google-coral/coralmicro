cmake_minimum_required(VERSION 3.13)
project(a71ch)

include_directories(.)

add_executable_m7(a71ch
    a71ch.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/testconv1-edgetpu.tflite
)

target_include_directories(a71ch PRIVATE
    .
)

target_link_libraries(a71ch
    libs_base-m7_freertos
    libs_a71ch
)

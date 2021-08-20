cmake_minimum_required(VERSION 3.13)
project(BFT)

include_directories(.)

add_executable_m7(BFT
    bft.cc
    M4_EXECUTABLE
    BFTM4
    DATA
    models/testconv1-edgetpu.tflite
    models/testconv1-expected-output.bin
    models/testconv1-test-input.bin
)

target_link_libraries(BFT
    libs_base-m7_freertos
    libs_tensorflow-m7
)

add_executable_m4(BFTM4
    bft_m4.cc
)

target_link_libraries(BFTM4
    libs_base-m4_freertos
)

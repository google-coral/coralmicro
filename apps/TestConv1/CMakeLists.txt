cmake_minimum_required(VERSION 3.13)
project(TestConv1)

add_executable_m7(TestConv1
    testconv1.cc
    DATA
    ${CMAKE_SOURCE_DIR}/models/testconv1-edgetpu.tflite
    ${CMAKE_SOURCE_DIR}/models/testconv1-expected-output.bin
    ${CMAKE_SOURCE_DIR}/models/testconv1-test-input.bin
)

target_include_directories(TestConv1 PRIVATE
    .
)

target_link_libraries(TestConv1
    libs_base-m7_freertos
    libs_tasks_EdgeTpuTask_freertos
    libs_tasks_EdgeTpuDfuTask_freertos
    libs_tasks_UsbHostTask_freertos
    libs_tensorflow-m7
    libs_tpu_freertos
)

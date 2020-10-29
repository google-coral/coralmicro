cmake_minimum_required(VERSION 3.13)
add_library(libs_CMSIS INTERFACE
)
target_include_directories(libs_CMSIS INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/CMSIS/CMSIS/Core/Include
)

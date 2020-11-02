cmake_minimum_required(VERSION 3.13)

add_library(libs_flatbuffers INTERFACE
)
target_include_directories(libs_flatbuffers INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/flatbuffers/include
)

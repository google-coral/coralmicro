cmake_minimum_required(VERSION 3.13)

add_library(libs_gemmlowp INTERFACE
)
target_include_directories(libs_gemmlowp INTERFACE
    ${CMAKE_SOURCE_DIR}/third_party/gemmlowp
)

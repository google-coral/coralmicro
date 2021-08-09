cmake_minimum_required(VERSION 3.13)
add_library_m7(libs_mjson STATIC
    ${CMAKE_SOURCE_DIR}/third_party/mjson/src/mjson.c
)

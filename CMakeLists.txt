cmake_minimum_required(VERSION 3.13)

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi-gcc.cmake)
endif()

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(valiant)
include_directories(.)
add_subdirectory(apps)
add_subdirectory(libs)

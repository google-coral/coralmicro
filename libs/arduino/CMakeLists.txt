cmake_minimum_required(VERSION 3.13)
add_library(libs_arduino STATIC
    CMakeLists.txt
)
target_link_libraries(libs_arduino
    libs_base-m7_freertos
)
bundle_static_library(libs_arduino libs_arduino_bundled)
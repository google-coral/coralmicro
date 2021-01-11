cmake_minimum_required(VERSION 3.13)
project(BootROM)

add_executable_m7(BootROM
    boot_rom.cc
)

target_include_directories(BootROM PRIVATE
    .
)

target_link_libraries(BootROM
    libs_base-m7_bm
)

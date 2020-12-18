cmake_minimum_required(VERSION 3.13)
project(BootROM)

add_executable_m7(BootROM
    boot_rom.cc
)

target_include_directories(BootROM PRIVATE
    .
)

target_link_libraries(BootROM
    libs_nxp_rt1176-sdk_bm
)
target_sources(BootROM PUBLIC $<TARGET_OBJECTS:libs_nxp_rt1176-sdk_bm>)

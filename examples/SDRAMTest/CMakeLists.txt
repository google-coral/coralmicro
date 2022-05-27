add_executable_m7(SDRAMTest
    sdram_test.cc
)

target_link_libraries(SDRAMTest
    libs_base-m7_freertos
)

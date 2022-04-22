add_executable_m7(FlexSPI_NAND
    flexspi_nand.cc
    flexspi_nand_lut.c
)

target_link_libraries(FlexSPI_NAND
    libs_base-m7_freertos
)

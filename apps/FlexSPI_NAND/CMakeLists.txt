cmake_minimum_required(VERSION 3.13)
project(FlexSPI_NAND)

include_directories(.)

add_executable_m7(FlexSPI_NAND
    flexspi_nand.cc
    flexspi_nand_lut.c
)

target_include_directories(FlexSPI_NAND PRIVATE
    .
)

target_link_libraries(FlexSPI_NAND
    libs_base-m7_freertos
)

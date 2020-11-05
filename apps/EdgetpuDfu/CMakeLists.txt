cmake_minimum_required(VERSION 3.13)
project(EdgetpuDfu)

add_executable(EdgetpuDfu
    apex_latest_single_ep_bin.c
    board.c
    clock_config.c
    dfu.cc
    edgetpu_dfu.cc
    evkmimxrt1170_flexspi_nor_config.c
    fsl_flexspi_nor_boot.c
    peripherals.c
    pin_mux.c
)

target_include_directories(EdgetpuDfu PRIVATE
    .
)

target_link_libraries(EdgetpuDfu
    libs_nxp_rt1176-sdk_bm
)

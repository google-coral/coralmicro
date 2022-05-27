add_executable_m7(EdgefastScan
    edgefast_scan.cc
)

target_link_libraries(EdgefastScan
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_wiced
    edgefast_bluetooth
)
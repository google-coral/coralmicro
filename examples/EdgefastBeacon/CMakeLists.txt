add_executable_m7(EdgefastBeacon
    edgefast_beacon.cc
)

target_link_libraries(EdgefastBeacon
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_wiced
    edgefast_bluetooth
)

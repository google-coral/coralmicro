add_executable_m7(ble_beacon
    ble_beacon.cc
)

target_link_libraries(ble_beacon
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_wiced
    edgefast_bluetooth
)

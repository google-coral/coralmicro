add_executable_m7(ble_beacon_scan
    ble_beacon_scan.cc
)

target_link_libraries(ble_beacon_scan
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_wiced
    edgefast_bluetooth
)

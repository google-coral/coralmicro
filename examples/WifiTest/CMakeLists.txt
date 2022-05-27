add_executable_m7(WifiTest
    wifi_test.cc
)

target_link_libraries(WifiTest
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_wiced
)

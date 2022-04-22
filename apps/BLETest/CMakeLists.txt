add_executable_m7(BLETest
    ble_hello_sensor.c
    ble_test.cc
    wiced_bt_cfg.c
)

target_include_directories(BLETest PRIVATE
    .
)

target_link_libraries(BLETest
    libs_base-m7_freertos
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/libwicedble.a
    libs_nxp_rt1176-sdk_wiced
)

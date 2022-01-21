cmake_minimum_required(VERSION 3.13)
project(MFGTest)

include_directories(.)

add_executable_m7(MFGTest
    mfg_test.cc
)

target_link_libraries(MFGTest
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_mjson
    libs_testconv1
    libs_testlib
    libs_tpu_freertos
)

add_executable_m7(MFGTestWiFi
    mfg_test_wifi.cc
    wiced_bt_cfg.c
)

target_link_libraries(MFGTestWiFi
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_mjson
    libs_testlib
    # put me before wiced include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/wiced_bt/BTE/libwicedble.a
    libs_nxp_rt1176-sdk_wiced
)

add_executable_m7(MFGTestPOE
    mfg_test_poe.cc
    ${VALIANT_SOURCE_DIR}/libs/base/ethernet.cc
)

target_link_libraries(MFGTestPOE
    libs_base-m7_freertos
    libs_RPCServer
    libs_RPCServerIOHTTP
    libs_mjson
    libs_testlib
)

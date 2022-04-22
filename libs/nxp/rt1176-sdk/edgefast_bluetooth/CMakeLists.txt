add_library_m7(edgefast_bluetooth_internal
    BT_storage_pl.cc
    bt_ble_settings.cc
    edgefast_bluetooth.cc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_a2dp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_addr.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_att.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_api.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_internal.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_buf.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_gatt.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hfp_ag.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys_br.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap_br.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rfcomm.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_sdp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp_null.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_spp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ssp.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_uuid.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/atomic_c.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/buf.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/porting.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/slist.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/work_queue.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_status_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ht_read_task_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/FunctionLib/FunctionLib.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib/SecLib.c
)

target_compile_options(edgefast_bluetooth_internal PRIVATE
    -include ${VALIANT_SOURCE_DIR}/libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth_config.h
)

target_include_directories(edgefast_bluetooth_internal PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/fatfs/source
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/eOSAL
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/include
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/att
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/avctp
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/avdtp
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/bnep
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/dbase
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/mcap
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/obex
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/sdp
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/sm
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/smp
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/at_parser
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/sbc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/osal/src/freertos
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/sbc
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/Common
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/FunctionLib
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib
)

target_compile_definitions(edgefast_bluetooth_internal PUBLIC
    CONFIG_ARM=1
    CFG_BLE
    CONFIG_BT_SMP
    CONFIG_BT_PERIPHERAL
    CONFIG_BT_SETTINGS
    CONFIG_BT_KEYS_OVERWRITE_OLDEST
    CONFIG_BT_PRIVACY
)

target_link_libraries(edgefast_bluetooth_internal PUBLIC
    libs_nxp_rt1176-sdk_freertos
    libs_nxp_rt1176-sdk_wiced
    libs_littlefs
)

add_library_m7(edgefast_bluetooth INTERFACE)
target_link_libraries(edgefast_bluetooth INTERFACE
    -Wl,--start-group
    edgefast_bluetooth_internal
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib/lib_crypto_m7.a
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_core.a
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_gatt.a
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_protocol.a
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_util.a
    -Wl,--end-group
)

# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_library_m7(edgefast_bluetooth_internal
    edgefast_bluetooth.cc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_a2dp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_addr.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_adv.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_att.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_api.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_avrcp_al_internal.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_br.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_buf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_conn.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_crypto.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_direction.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ecc.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_gatt.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hci_core.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_hfp_ag.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_id.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_keys_br.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_l2cap_br.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_monitor.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rfcomm.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_rpa.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_scan.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_sdp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_settings.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_smp_null.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_spp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_ssp.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/host/bt_pal_uuid.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_platform.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/atomic_c.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/buf.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/porting.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/slist.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting/work_queue.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/BT_common_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/aes_cmac_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/device_queue_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension/smp_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/osal/src/freertos/EM_timer.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_status_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/BT_storage_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/btsnoop_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/hci_uart.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/ht_read_task_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/sco_audio_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/write_task_pl.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_debug.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal/ethal_os.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/FunctionLib/FunctionLib.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib/SecLib.c
)

target_compile_options(edgefast_bluetooth_internal PRIVATE
    -include ${PROJECT_SOURCE_DIR}/third_party/modified/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth_config.h
)

target_include_directories(edgefast_bluetooth_internal PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/configs
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/porting
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/fatfs/source
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/eOSAL
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/extension
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/include
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/config
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/att
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/avctp
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/avdtp
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/bnep
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/dbase
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/hci_1.2
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/mcap
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/obex
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/rfcomm
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/sdp
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/sm
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/protocols/smp
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/aes_cmac
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/at_parser
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/utils/sbc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/osal/src/freertos
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/ethal
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/sbc
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/Common
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/FunctionLib
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib
)

target_compile_definitions(edgefast_bluetooth_internal PUBLIC
    CONFIG_ARM=1
    CFG_BLE
    CONFIG_BT_SMP
    CONFIG_BT_PERIPHERAL
    CONFIG_BT_SETTINGS
    CONFIG_BT_KEYS_OVERWRITE_OLDEST
    CONFIG_BT_PRIVACY
    CONFIG_BT_OBSERVER
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
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/framework/SecLib/lib_crypto_m7.a
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_core.a
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_gatt.a
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_protocol.a
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/private/lib/mcux/default/ble/cm7f/gcc/libethermind_ble_util.a
    -Wl,--end-group
)

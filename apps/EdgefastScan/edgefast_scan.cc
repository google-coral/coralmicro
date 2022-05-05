// Port of Zephyr's beacon sample.
// Original sample at:
// https://github.com/zephyrproject-rtos/zephyr/blob/main/samples/bluetooth/scan_adv/src/main.c

#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

static void scan_cb(const bt_addr_le_t* addr, int8_t rssi, uint8_t adv_type,
                    struct net_buf_simple* buf) {
    static char addr_s[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_s, sizeof(addr_s));

    printf("Scan: find device %s\r\n", addr_s);
}

static void bt_ready(int err) {
    struct bt_le_scan_param scan_param = {
        .type = BT_HCI_LE_SCAN_ACTIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = 0x0800,
        .window = 0x0010,
    };

    err = bt_le_scan_start(&scan_param, scan_cb);
    if (err) {
        printf("Starting scanning failed (err %d)\n", err);
        return;
    }
}

extern "C" void app_main(void* param) {
    InitEdgefastBluetooth(bt_ready);
    vTaskSuspend(NULL);
}

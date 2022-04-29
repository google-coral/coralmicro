#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/wwd_wiced.h"
#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/port/pal/mcux/bluetooth/controller.h"
#include "third_party/nxp/rt1176-sdk/middleware/wireless/ethermind/bluetooth/export/include/BT_hci_api.h"
#include "third_party/nxp/rt1176-sdk/middleware/edgefast_bluetooth/source/impl/ethermind/platform/bt_ble_settings.h"

extern unsigned char brcm_patchram_buf[];
extern unsigned int brcm_patch_ram_length;
extern "C" wiced_result_t wiced_wlan_connectivity_init(void);
void ble_pwr_on(void);

extern "C" int controller_hci_uart_get_configuration(controller_hci_uart_config_t *config) {
    if (!config) {
        return -1;
    }
    config->clockSrc = CLOCK_GetRootClockFreq(kCLOCK_Root_Lpuart2);
    config->defaultBaudrate = 115200;
    config->runningBaudrate = 115200;
    config->instance = 2;
    config->enableRxRTS = 1;
    config->enableTxCTS = 1;
    return 0;
}

static bt_ready_cb_t g_cb = nullptr;
static void bt_ready_internal(int err_param) {
    if (err_param) {
        printf("Bluetooth initialization failed: %d\r\n", err_param);
        return;
    }

    // Kick the Bluetooth module into patchram download mode.
    constexpr int kCmdDownloadMode = 0x2E;
    int err = bt_hci_cmd_send_sync(BT_OP(BT_OGF_VS, kCmdDownloadMode), NULL, NULL);
    if (err) {
        printf("Initializing patchram download failed: %d\r\n", err);
        return;
    }
    // Sleep to allow the transition into download mode.
    vTaskDelay(pdMS_TO_TICKS(50));

    // The patchram file consists of raw HCI commands.
    // Build command buffers and send them to the module.
    size_t offset = 0;
    while (offset != brcm_patch_ram_length) {
        uint16_t opcode = *(uint16_t*)&brcm_patchram_buf[offset];
        uint8_t len = brcm_patchram_buf[offset + sizeof(uint16_t)];
        offset += sizeof(uint16_t) + sizeof(uint8_t);
        struct net_buf *buf = bt_hci_cmd_create(opcode, len);
        uint8_t* dat = reinterpret_cast<uint8_t*>(net_buf_add(buf, len));
        memcpy(dat, &brcm_patchram_buf[offset], len);
        offset += len;

        err = bt_hci_cmd_send_sync(opcode, buf, NULL);
        net_buf_unref(buf);
        if (err) {
            printf("Sending patchram packet failed: %d\r\n", err);
            return;
        }
    }
    // Sleep to let the patched firmware execute.
    vTaskDelay(pdMS_TO_TICKS(200));

    if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
        settings_load();
    }

    if (g_cb) {
        g_cb(err);
    }
}

void InitEdgefastBluetooth(bt_ready_cb_t cb) {
    if (coral::micro::filesystem::ReadFile("/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd",
                                      brcm_patchram_buf,
                                      brcm_patch_ram_length) != brcm_patch_ram_length) {
        printf("Reading patchram failed\r\n");
        assert(false);
    }
    wiced_wlan_connectivity_init();
    coral::micro::gpio::SetGpio(coral::micro::gpio::kBtDevWake, false);
    ble_pwr_on();
    g_cb = cb;
    int err = bt_enable(bt_ready_internal);
    if (err) {
        printf("bt_enable failed\r\n");
    }
}

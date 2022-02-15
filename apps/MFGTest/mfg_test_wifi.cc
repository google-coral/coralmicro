#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/main_freertos_m7.h"
#include "libs/base/mutex.h"
#include "libs/testlib/test_lib.h"
#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/semphr.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_BLE/bt_app_inc/wiced_bt_stack.h"
#include "third_party/nxp/rt1176-sdk/middleware/wiced/43xxx_Wi-Fi/WICED/WWD/wwd_wiced.h"
extern "C" {
#include "libs/nxp/rt1176-sdk/rtos/freertos/libraries/abstractions/wifi/include/iot_wifi.h"
}

extern const wiced_bt_cfg_settings_t wiced_bt_cfg_settings;
extern const wiced_bt_cfg_buf_pool_t wiced_bt_cfg_buf_pools[];
static WIFIReturnCode_t xWifiStatus = eWiFiFailure;

static void WifiGetAP(struct jsonrpc_request *request) {
    if (xWifiStatus != eWiFiSuccess) {
        jsonrpc_return_error(request, -1, "wifi failed to initialize", nullptr);
        return;
    }

    const char *name_param_pattern = "$[0].name";
    ssize_t size = 0;
    int find_result;

    std::vector<char> name;
    find_result = mjson_find(request->params, request->params_len, name_param_pattern, nullptr, &size);
    if (find_result == MJSON_TOK_STRING) {
        name.resize(size, 0);
        mjson_get_string(request->params, request->params_len, name_param_pattern, name.data(), size);
    } else {
        jsonrpc_return_error(request, -1, "'name' missing or invalid", nullptr);
        return;
    }

    WIFIReturnCode_t wifi_ret;
    constexpr int kNumResults = 50;
    WIFIScanResult_t xScanResults[kNumResults] = {0};
    wifi_ret = WIFI_Scan(xScanResults, kNumResults);

    if (wifi_ret != eWiFiSuccess) {
        jsonrpc_return_error(request, -1, "wifi scan failed", nullptr);
        return;
    }

    std::vector<int> scan_indices;
    for (int i = 0; i < kNumResults; ++i) {
        if (memcmp(xScanResults[i].cSSID, name.data(), size) == 0) {
            scan_indices.push_back(i);
        }
    }

    if (scan_indices.size() == 0) {
        jsonrpc_return_error(request, -1, "network not found", nullptr);
        return;
    }

    int8_t best_rssi = SCHAR_MIN;
    for (unsigned int i = 0; i < scan_indices.size(); ++i) {
        if (xScanResults[scan_indices[i]].cRSSI > best_rssi) {
            best_rssi = xScanResults[scan_indices[i]].cRSSI;
        }
    }

    jsonrpc_return_success(request, "{%Q:%d}", "signal_strength", best_rssi);
}

static SemaphoreHandle_t ble_ready_mtx;
static bool ble_ready = false;
static SemaphoreHandle_t ble_scan_sema;
static void BLEFind(struct jsonrpc_request *request) {
    {
        valiant::MutexLock lock(ble_ready_mtx);
        if (!ble_ready) {
            jsonrpc_return_error(request, -1, "bt not ready yet", nullptr);
            return;
        }
    }

    const char *address_param_pattern = "$[0].address";
    std::vector<char> address;
    int size;
    int find_result;
    find_result = mjson_find(request->params, request->params_len, address_param_pattern, nullptr, &size);
    if (find_result == MJSON_TOK_STRING) {
        address.resize(size + 1, 0);
        mjson_get_string(request->params, request->params_len, address_param_pattern, address.data(), size);
    } else {
        jsonrpc_return_error(request, -1, "'address' missing", nullptr);
        return;
    }

    unsigned int a, b, c, d, e, f;
    int tokens = sscanf(address.data(), "%02X:%02X:%02X:%02X:%02X:%02X", &a, &b, &c, &d, &e, &f);
    if (tokens != 6) {
        jsonrpc_return_error(request, -1, "could not get six octets from 'address'", nullptr);
        return;
    }
    static int8_t rssi;
    static wiced_bt_device_address_t search_address;
    search_address[0] = static_cast<uint8_t>(a);
    search_address[1] = static_cast<uint8_t>(b);
    search_address[2] = static_cast<uint8_t>(c);
    search_address[3] = static_cast<uint8_t>(d);
    search_address[4] = static_cast<uint8_t>(e);
    search_address[5] = static_cast<uint8_t>(f);

    // This static here is because there isn't a way to pass a parameter into the scan.
    // Reset the value each time through the method.
    static bool found_match;
    found_match = false;
    wiced_result_t ret = wiced_bt_ble_observe(WICED_TRUE, 3, [](wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data) {
        if (p_scan_result) {
            if (memcmp(search_address, p_scan_result->remote_bd_addr, sizeof(wiced_bt_device_address_t)) == 0) {
                found_match = true;
                rssi = p_scan_result->rssi;
            }
        } else {
            xSemaphoreGive(ble_scan_sema);
        }
    });
    if (ret != WICED_BT_PENDING) {
        jsonrpc_return_error(request, -1, "failed to initiate bt scan", nullptr);
        return;
    }
    xSemaphoreTake(ble_scan_sema, portMAX_DELAY);
    if (found_match) {
        jsonrpc_return_success(request, "{%Q:%d}", "signal_strength", rssi);
    } else {
        jsonrpc_return_error(request, -1, "failed to find 'address'", nullptr);
    }
}

static void BLEScan(struct jsonrpc_request *request) {
    {
        valiant::MutexLock lock(ble_ready_mtx);
        if (!ble_ready) {
            jsonrpc_return_error(request, -1, "bt not ready yet", nullptr);
            return;
        }
    }

    static int8_t rssi;
    static char address[18] = "00:00:00:00:00:00";

    // This static here is because there isn't a way to pass a parameter into the scan.
    // Reset the value each time through the method.
    static bool found_address;
    found_address = false;
    wiced_result_t ret = wiced_bt_ble_observe(WICED_TRUE, 3, [](wiced_bt_ble_scan_results_t *p_scan_result, uint8_t *p_adv_data) {
        if (p_scan_result) {
            found_address = true;
            auto s = p_scan_result->remote_bd_addr;
            sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X", s[0], s[1], s[2], s[3], s[4], s[5]);
            rssi = p_scan_result->rssi;
        } else {
            xSemaphoreGive(ble_scan_sema);
        }
    });
    if (ret != WICED_BT_PENDING) {
        jsonrpc_return_error(request, -1, "failed to initiate bt scan", nullptr);
        return;
    }
    xSemaphoreTake(ble_scan_sema, portMAX_DELAY);
    if (found_address) {
        jsonrpc_return_success(request, "{%Q:%Q, %Q:%d}", "address", address, "signal_strength", rssi);
    } else {
        jsonrpc_return_error(request, -1, "failed to find 'address'", nullptr);
    }
}


static wiced_result_t ble_management_callback(wiced_bt_management_evt_t event,
                                                       wiced_bt_management_evt_data_t *p_event_data) {
    switch (event) {
        case BTM_ENABLED_EVT:
            {
                valiant::MutexLock lock(ble_ready_mtx);
                if (((wiced_bt_dev_enabled_t*)(p_event_data))->status == WICED_SUCCESS) {
                    ble_ready = true;
                } else {
                    ble_ready = false;
                }
            }
            break;
        case BTM_LPM_STATE_LOW_POWER:
            break;
        default:
            return WICED_BT_ERROR;
            break;
    }
    return WICED_BT_SUCCESS;
}

extern unsigned char brcm_patchram_buf[];
extern unsigned int brcm_patch_ram_length;
extern "C" void app_main(void *param) {
    ble_scan_sema = xSemaphoreCreateBinary();
    ble_ready_mtx = xSemaphoreCreateMutex();
    size_t brcm_patchram_size = brcm_patch_ram_length;
    if (!valiant::filesystem::ReadToMemory("/third_party/cyw-bt-patch/BCM4345C0_003.001.025.0144.0266.1MW.hcd",
                                            brcm_patchram_buf,
                                            &brcm_patchram_size)) {
        printf("Reading patchram failed\r\n");
        vTaskSuspend(NULL);
    }

    xWifiStatus = WIFI_On();
    valiant::gpio::SetGpio(valiant::gpio::kBtDevWake, false);
    wiced_bt_stack_init(ble_management_callback, &wiced_bt_cfg_settings, wiced_bt_cfg_buf_pools);

    valiant::rpc::RPCServerIOHTTP rpc_server_io_http;
    valiant::rpc::RPCServer rpc_server;
    if (!rpc_server_io_http.Init()) {
        printf("Failed to initialize RPCServerIOHTTP\r\n");
        vTaskSuspend(NULL);
    }
    if (!rpc_server.Init()) {
        printf("Failed to initialize RPCServer\r\n");
        vTaskSuspend(NULL);
    }
    rpc_server.RegisterIO(rpc_server_io_http);

    rpc_server.RegisterRPC("wifi_get_ap", WifiGetAP);
    rpc_server.RegisterRPC(valiant::testlib::kWifiSetAntennaName, valiant::testlib::WifiSetAntenna);
    rpc_server.RegisterRPC("ble_scan", BLEScan);
    rpc_server.RegisterRPC("ble_find", BLEFind);

    vTaskSuspend(NULL);
}

extern "C" int main(int argc, char **argv) {
    return real_main(argc, argv, false, false);
}

#include "libs/RPCServer/rpc_server.h"
#include "libs/RPCServer/rpc_server_io_http.h"
#include "libs/base/analog.h"
#include "libs/base/filesystem.h"
#include "libs/base/gpio.h"
#include "libs/base/utils.h"
#include "libs/tasks/AudioTask/audio_task.h"
#include "libs/tasks/CameraTask/camera_task.h"
#include "libs/tasks/EdgeTpuTask/edgetpu_task.h"
#include "libs/tasks/PmicTask/pmic_task.h"
#include "libs/testconv1/testconv1.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/middleware/mbedtls/include/mbedtls/base64.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"

#include <array>
#include <map>

// In the below maps, data about pins to be tested via the loopback fixture
// is provided. Pins on J5 are numbered from 1-100, and pins on J6 are numbered from 101-200.

// Map from pin-to-pin: Used to validate that the provided
// pair will be connected via loopback fixture.
static std::map<int, int> j5_j6_loopback_mapping;

// IOMUXC entries for each pin to be tested.
static std::map<int, std::array<uint32_t, 5>> j5_j6_iomuxc;

// GPIO module / pin number for each pin to be tested.
static std::map<int, std::pair<GPIO_Type*, int>> j5_j6_gpio_pins;

static void InitializeLoopbackMappings() {
    j5_j6_loopback_mapping = {
        {1, 3},
        {3, 1},
        {5, 6},
        {6, 5},
        {7, 9},
        {9, 7},
        {10, 178},
        {14, 95},
        {15, 17},
        {17, 15},
        {33, 34},
        {34, 33},
        {35, 72},
        {40, 42},
        {41, 44},
        {42, 40},
        {43, 45},
        {44, 1},
        {45, 43},
        {46, 48},
        {48, 46},
        {50, 52},
        {51, 53},
        {52, 50},
        {53, 51},
        {56, 58},
        {58, 56},
        {62, 64},
        {64, 62},
        {66, 68},
        {68, 66},
        {71, 73},
        {72, 35},
        {73, 71},
        {75, 77},
        {77, 75},
        {78, 82},
        {79, 81},
        {81, 79},
        {82, 78},
        {85, 87},
        {86, 88},
        {87, 85},
        {88, 86},
        {89, 91},
        {91, 89},
        {92, 94},
        {93, 100},
        {94, 92},
        {96, 98},
        {98, 96},
        {100, 93},
        {140, 142},
        {142, 140},
        {144, 146},
        {146, 144},
        {150, 152},
        {152, 150},
        {154, 156},
        {155, 157},
        {156, 154},
        {157, 155},
        {158, 160},
        {159, 161},
        {160, 158},
        {161, 159},
        {162, 166},
        {163, 165},
        {165, 163},
        {166, 162},
        {168, 170},
        {169, 171},
        {170, 168},
        {171, 169},
        {172, 176},
        {173, 175},
        {175, 173},
        {176, 172},
        {178, 10},
    };
    j5_j6_iomuxc = {
        {1, {IOMUXC_GPIO_AD_27_GPIO9_IO26}},
        {3, {IOMUXC_GPIO_AD_27_GPIO9_IO26}},
        {5, {IOMUXC_GPIO_AD_03_GPIO9_IO02}},
        {6, {IOMUXC_GPIO_AD_11_GPIO9_IO10}},
        {7, {IOMUXC_GPIO_AD_01_GPIO9_IO00}},
        {9, {IOMUXC_GPIO_AD_00_GPIO8_IO31}},
        {10, {IOMUXC_GPIO_AD_06_GPIO9_IO05}},
        {14, {IOMUXC_GPIO_AD_07_GPIO9_IO06}},
        {15, {IOMUXC_GPIO_LPSR_04_GPIO12_IO04}},
        {17, {IOMUXC_GPIO_LPSR_05_GPIO12_IO05}},
        {33, {IOMUXC_GPIO_SNVS_05_DIG_GPIO13_IO08}},
        {34, {IOMUXC_GPIO_SNVS_07_DIG_GPIO13_IO10}},
        {35, {IOMUXC_GPIO_LPSR_07_GPIO12_IO07}},
        {40, {IOMUXC_GPIO_EMC_B2_10_GPIO8_IO20}},
        {41, {IOMUXC_GPIO_LPSR_14_GPIO12_IO14}},
        {42, {IOMUXC_GPIO_EMC_B2_06_GPIO8_IO16}},
        {43, {IOMUXC_GPIO_LPSR_13_GPIO12_IO13}},
        {44, {IOMUXC_GPIO_LPSR_12_GPIO12_IO12}},
        {45, {IOMUXC_GPIO_LPSR_01_GPIO12_IO01}},
        {46, {IOMUXC_GPIO_LPSR_11_GPIO12_IO11}},
        {48, {IOMUXC_GPIO_LPSR_10_GPIO12_IO10}},
        {50, {IOMUXC_GPIO_LPSR_09_GPIO12_IO09}},
        {51, {IOMUXC_GPIO_AD_32_GPIO9_IO31}},
        {52, {IOMUXC_GPIO_AD_33_GPIO10_IO00}},
        {53, {IOMUXC_GPIO_EMC_B2_08_GPIO8_IO18}},
        {56, {IOMUXC_GPIO_SNVS_08_DIG_GPIO13_IO11}},
        {58, {IOMUXC_GPIO_SNVS_04_DIG_GPIO13_IO07}},
        {62, {IOMUXC_GPIO_EMC_B2_05_GPIO8_IO15}},
        {64, {IOMUXC_GPIO_EMC_B2_09_GPIO8_IO19}},
        {66, {IOMUXC_GPIO_EMC_B2_04_GPIO8_IO14}},
        {68, {IOMUXC_GPIO_EMC_B2_07_GPIO8_IO17}},
        {71, {IOMUXC_GPIO_SD_B2_00_GPIO10_IO09}},
        {72, {IOMUXC_GPIO_LPSR_06_GPIO12_IO06}},
        {73, {IOMUXC_GPIO_SD_B2_02_GPIO10_IO11}},
        {75, {IOMUXC_GPIO_SD_B2_01_GPIO10_IO10}},
        {77, {IOMUXC_GPIO_SD_B2_03_GPIO10_IO12}},
        {78, {IOMUXC_GPIO_SNVS_06_DIG_GPIO13_IO09}},
        {79, {IOMUXC_GPIO_SD_B2_04_GPIO10_IO13}},
        {81, {IOMUXC_GPIO_SD_B2_05_GPIO10_IO14}},
        {82, {IOMUXC_GPIO_DISP_B2_09_GPIO11_IO10}},
        {85, {IOMUXC_GPIO_SD_B1_04_GPIO10_IO07}},
        {86, {IOMUXC_GPIO_DISP_B2_07_GPIO11_IO08}},
        {87, {IOMUXC_GPIO_AD_35_GPIO10_IO02}},
        {88, {IOMUXC_GPIO_DISP_B2_07_GPIO11_IO08}},
        {89, {IOMUXC_GPIO_AD_24_GPIO9_IO23}},
        {91, {IOMUXC_GPIO_AD_25_GPIO9_IO24}},
        {92, {IOMUXC_GPIO_SD_B1_01_GPIO10_IO04}},
        {93, {IOMUXC_GPIO_AD_34_GPIO10_IO01}},
        {94, {IOMUXC_GPIO_SD_B1_03_GPIO10_IO06}},
        {96, {IOMUXC_GPIO_SD_B1_02_GPIO10_IO05}},
        {98, {IOMUXC_GPIO_SD_B1_05_GPIO10_IO08}},
        {100, {IOMUXC_GPIO_SD_B1_00_GPIO10_IO03}},
        {140, {IOMUXC_GPIO_EMC_B2_02_GPIO8_IO12}},
        {142, {IOMUXC_GPIO_EMC_B2_03_GPIO8_IO13}},
        {144, {IOMUXC_GPIO_EMC_B2_19_GPIO8_IO29}},
        {146, {IOMUXC_GPIO_EMC_B2_20_GPIO8_IO30}},
        {150, {IOMUXC_GPIO_DISP_B2_15_GPIO11_IO16}},
        {152, {IOMUXC_GPIO_DISP_B2_08_GPIO11_IO09}},
        {154, {IOMUXC_GPIO_DISP_B2_13_GPIO11_IO14}},
        {155, {IOMUXC_GPIO_DISP_B1_06_GPIO10_IO27}},
        {156, {IOMUXC_GPIO_DISP_B2_12_GPIO11_IO13}},
        {157, {IOMUXC_GPIO_DISP_B1_07_GPIO10_IO28}},
        {158, {IOMUXC_GPIO_DISP_B2_10_GPIO11_IO11}},
        {159, {IOMUXC_GPIO_DISP_B1_08_GPIO10_IO29}},
        {160, {IOMUXC_GPIO_DISP_B2_11_GPIO11_IO12}},
        {161, {IOMUXC_GPIO_DISP_B1_09_GPIO10_IO30}},
        {162, {IOMUXC_GPIO_DISP_B2_14_GPIO11_IO15}},
        {163, {IOMUXC_GPIO_DISP_B1_10_GPIO10_IO31}},
        {165, {IOMUXC_GPIO_DISP_B1_11_GPIO11_IO00}},
        {166, {IOMUXC_GPIO_DISP_B2_04_GPIO11_IO05}},
        {168, {IOMUXC_GPIO_DISP_B2_03_GPIO11_IO04}},
        {169, {IOMUXC_GPIO_AD_28_GPIO9_IO27}},
        {170, {IOMUXC_GPIO_DISP_B2_02_GPIO11_IO03}},
        {171, {IOMUXC_GPIO_AD_29_GPIO9_IO28}},
        {172, {IOMUXC_GPIO_DISP_B2_05_GPIO11_IO06}},
        {173, {IOMUXC_GPIO_AD_30_GPIO9_IO29}},
        {175, {IOMUXC_GPIO_AD_31_GPIO9_IO30}},
        {176, {IOMUXC_GPIO_DISP_B2_00_GPIO11_IO01}},
        {178, {IOMUXC_GPIO_DISP_B2_01_GPIO11_IO02}},
    };

    j5_j6_gpio_pins = {
        {1, {GPIO9, 26}},
        {3, {GPIO9, 26}},
        {5, {GPIO9, 2}},
        {6, {GPIO9, 10}},
        {7, {GPIO9, 0}},
        {9, {GPIO8, 31}},
        {10, {GPIO9, 5}},
        {14, {GPIO9, 6}},
        {15, {GPIO12, 4}},
        {17, {GPIO12, 5}},
        {33, {GPIO13, 8}},
        {34, {GPIO13, 10}},
        {35, {GPIO12, 7}},
        {40, {GPIO8, 20}},
        {41, {GPIO12, 14}},
        {42, {GPIO8, 16}},
        {43, {GPIO12, 13}},
        {44, {GPIO12, 12}},
        {45, {GPIO12, 1}},
        {46, {GPIO12, 11}},
        {48, {GPIO12, 10}},
        {50, {GPIO12, 9}},
        {51, {GPIO9, 31}},
        {52, {GPIO10, 0}},
        {53, {GPIO8, 18}},
        {56, {GPIO13, 11}},
        {58, {GPIO13, 7}},
        {62, {GPIO8, 15}},
        {64, {GPIO8, 19}},
        {66, {GPIO8, 14}},
        {68, {GPIO8, 17}},
        {71, {GPIO10, 9}},
        {72, {GPIO12, 6}},
        {73, {GPIO10, 11}},
        {75, {GPIO10, 10}},
        {77, {GPIO10, 12}},
        {78, {GPIO13, 9}},
        {79, {GPIO10, 13}},
        {81, {GPIO10, 14}},
        {82, {GPIO11, 10}},
        {85, {GPIO10, 7}},
        {86, {GPIO11, 8}},
        {87, {GPIO10, 2}},
        {88, {GPIO11, 8}},
        {89, {GPIO9, 23}},
        {91, {GPIO9, 24}},
        {92, {GPIO10, 4}},
        {93, {GPIO10, 1}},
        {94, {GPIO10, 6}},
        {96, {GPIO10, 5}},
        {98, {GPIO10, 8}},
        {100, {GPIO10, 3}},
        {140, {GPIO8, 12}},
        {142, {GPIO8, 13}},
        {144, {GPIO8, 29}},
        {146, {GPIO8, 30}},
        {150, {GPIO11, 16}},
        {152, {GPIO11, 9}},
        {154, {GPIO11, 14}},
        {155, {GPIO10, 27}},
        {156, {GPIO11, 13}},
        {157, {GPIO10, 28}},
        {158, {GPIO11, 11}},
        {159, {GPIO10, 29}},
        {160, {GPIO11, 12}},
        {161, {GPIO10, 30}},
        {162, {GPIO11, 15}},
        {163, {GPIO10, 31}},
        {165, {GPIO11, 0}},
        {166, {GPIO11, 5}},
        {168, {GPIO11, 4}},
        {169, {GPIO9, 27}},
        {170, {GPIO11, 3}},
        {171, {GPIO9, 28}},
        {172, {GPIO11, 6}},
        {173, {GPIO9, 29}},
        {175, {GPIO9, 30}},
        {176, {GPIO11, 1}},
        {178, {GPIO11, 2}},
    };
}

// Implementation of "get_serial_number" RPC.
// Returns JSON results with the key "serial_number" and the serial, as a string.
static void GetSerialNumber(struct jsonrpc_request *request) {
    char serial_number_str[16];
    uint64_t serial_number = valiant::utils::GetUniqueID();
    for (int i = 0; i < 16; ++i) {
        uint8_t nibble = (serial_number >> (i * 4)) & 0xF;
        if (nibble < 10) {
            serial_number_str[15 - i] = nibble + '0';
        } else {
            serial_number_str[15 - i] = (nibble - 10) + 'a';
        }
    }
    jsonrpc_return_success(request, "{%Q:%.*Q}", "serial_number", 16, serial_number_str);
}

// Implementation of "set_pmic_rail_state" RPC.
// Takes two parameters:
//    "rail" is an enumerated value indicating the rail to change.
//    "enable" is a boolean state to set the rail to.
// Returns success or failure to set the requested state.
static void SetPmicRailState(struct jsonrpc_request *request) {
    double rail_double;
    valiant::pmic::Rail rail;
    int enable;
    const char *rail_param_pattern = "$[0].rail";
    const char *enable_param_pattern = "$[0].enable";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, rail_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, rail_param_pattern, &rail_double);
        rail = static_cast<valiant::pmic::Rail>(static_cast<uint8_t>(rail_double));
    } else {
        jsonrpc_return_error(request, -1, "'rail' missing or invalid", nullptr);
        return;
    }
    find_result = mjson_find(request->params, request->params_len, enable_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        mjson_get_bool(request->params, request->params_len, enable_param_pattern, &enable);
    } else {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    valiant::PmicTask::GetSingleton()->SetRailState(rail, enable);
    jsonrpc_return_success(request, "{}");
}

// Implementation of "set_led_state" RPC.
// Takes two parameters:
//    "led" is an enumerated value indicating the LED to change.
//    "enable" is a boolean state to set the rail to.
// Returns success or failure to set the requested state.
// NOTE: The TPU LED requires that the TPU power is enabled.
static void SetLedState(struct jsonrpc_request *request) {
    double led_double;
    int enable, led;
    const char *led_param_pattern = "$[0].led";
    const char *enable_param_pattern = "$[0].enable";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, led_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, led_param_pattern, &led_double);
        led = static_cast<int>(led_double);
    } else {
        jsonrpc_return_error(request, -1, "'led' missing or invalid", nullptr);
        return;
    }
    find_result = mjson_find(request->params, request->params_len, enable_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        mjson_get_bool(request->params, request->params_len, enable_param_pattern, &enable);
    } else {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    enum LEDs {
        kPower = 0,
        kUser = 1,
        kTpu = 2,
    };
    switch (led) {
        case kPower:
            valiant::gpio::SetGpio(valiant::gpio::kPowerLED, enable);
            break;
        case kUser:
            valiant::gpio::SetGpio(valiant::gpio::kUserLED, enable);
            break;
        case kTpu:
            valiant::gpio::SetGpio(valiant::gpio::kTpuLED, enable);
            break;
        default:
            jsonrpc_return_error(request, -1, "invalid led", nullptr);
            return;
    }
    jsonrpc_return_success(request, "{}");
}

// Implements "set_pin_pair_to_gpio" request.
// Takes two parameters:
//    "input_pin" is the pin which will be set to input mode
//    "output_pin" is the pin which will be set to output mode
// Returns success or failure to set the pin states.
static void SetPinPairToGpio(struct jsonrpc_request *request) {
    double output_pin_double, input_pin_double;
    int output_pin, input_pin;
    const char *output_pin_param_pattern = "$[0].output_pin";
    const char *input_pin_param_pattern = "$[0].input_pin";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, output_pin_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, output_pin_param_pattern, &output_pin_double);
        output_pin = static_cast<int>(output_pin_double);
    } else {
        jsonrpc_return_error(request, -1, "'output_pin' missing", nullptr);
        return;
    }

    find_result = mjson_find(request->params, request->params_len, input_pin_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, input_pin_param_pattern, &input_pin_double);
        input_pin = static_cast<int>(input_pin_double);
    } else {
        jsonrpc_return_error(request, -1, "'input_pin' missing", nullptr);
        return;
    }

    auto pin_pair_a = j5_j6_loopback_mapping.find(output_pin);
    auto pin_pair_b = j5_j6_loopback_mapping.find(input_pin);
    if (pin_pair_a == j5_j6_loopback_mapping.end()) {
        jsonrpc_return_error(request, -1, "invalid 'output_pin'", nullptr);
        return;
    }
    if (pin_pair_b == j5_j6_loopback_mapping.end()) {
        jsonrpc_return_error(request, -1, "invalid 'input_pin'", nullptr);
        return;
    }
    if (pin_pair_a->first != pin_pair_b->second || pin_pair_b->first != pin_pair_a->second) {
        jsonrpc_return_error(request, -1, "'output_pin and input_pin are not a pair'", "{%Q:%d, %Q:%d}", "output_pin", output_pin, "input_pin", input_pin);
        return;
    }

    auto output_pin_mux = j5_j6_iomuxc.find(output_pin);
    auto input_pin_mux = j5_j6_iomuxc.find(input_pin);
    if (output_pin_mux == j5_j6_iomuxc.end()) {
        jsonrpc_return_error(request, -1, "'output_pin' mux settings not found", nullptr);
        return;
    }
    if (input_pin_mux == j5_j6_iomuxc.end()) {
        jsonrpc_return_error(request, -1, "'input_pin' mux settings not found", nullptr);
        return;
    }
    auto output_pin_gpio_values = j5_j6_gpio_pins.find(output_pin);
    if (output_pin_gpio_values == j5_j6_gpio_pins.end()) {
        jsonrpc_return_error(request, -1, "'output_pin' gpio settings not found", nullptr);
        return;
    }
    auto input_pin_gpio_values = j5_j6_gpio_pins.find(input_pin);
    if (input_pin_gpio_values == j5_j6_gpio_pins.end()) {
        jsonrpc_return_error(request, -1, "'input_pin' gpio settings not found", nullptr);
        return;
    }

    gpio_pin_config_t pin_config_output = {
        .direction = kGPIO_DigitalOutput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    };
    gpio_pin_config_t pin_config_input = {
        .direction = kGPIO_DigitalInput,
        .outputLogic = 0,
        .interruptMode = kGPIO_NoIntmode,
    };
    GPIO_PinInit(output_pin_gpio_values->second.first, output_pin_gpio_values->second.second, &pin_config_output);
    GPIO_PinInit(input_pin_gpio_values->second.first, input_pin_gpio_values->second.second, &pin_config_input);
    const uint32_t kInputBufferOn = 1U;
    IOMUXC_SetPinMux(output_pin_mux->second[0], output_pin_mux->second[1], output_pin_mux->second[2], output_pin_mux->second[3], output_pin_mux->second[4], kInputBufferOn);
    IOMUXC_SetPinMux(input_pin_mux->second[0], input_pin_mux->second[1], input_pin_mux->second[2], input_pin_mux->second[3], input_pin_mux->second[4], kInputBufferOn);
    const uint32_t kDisablePulls = 0U;
    IOMUXC_SetPinConfig(output_pin_mux->second[0], output_pin_mux->second[1], output_pin_mux->second[2], output_pin_mux->second[3], output_pin_mux->second[4], kDisablePulls);
    IOMUXC_SetPinConfig(input_pin_mux->second[0], input_pin_mux->second[1], input_pin_mux->second[2], input_pin_mux->second[3], input_pin_mux->second[4], kDisablePulls);

    jsonrpc_return_success(request, "{}");
}

// Implements the "set_gpio" RPC.
// Takes two parameters:
//    "pin" is the numerical value of the pin to set a state for.
//    "enable" is whether to drive the pin high or low.
// Returns success or failure.
static void SetGpio(struct jsonrpc_request *request) {
    double pin_double;
    int pin, enable;

    const char *pin_param_pattern = "$[0].pin";
    const char *enable_param_pattern = "$[0].enable";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, pin_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, pin_param_pattern, &pin_double);
        pin = static_cast<int>(pin_double);
    } else {
        jsonrpc_return_error(request, -1, "'pin' missing", nullptr);
        return;
    }

    find_result = mjson_find(request->params, request->params_len, enable_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        mjson_get_bool(request->params, request->params_len, enable_param_pattern, &enable);
    } else {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    auto pin_gpio_values = j5_j6_gpio_pins.find(pin);
    if (pin_gpio_values == j5_j6_gpio_pins.end()) {
        jsonrpc_return_error(request, -1, "invalid pin", nullptr);
        return;
    }
    GPIO_PinWrite(pin_gpio_values->second.first, pin_gpio_values->second.second, enable);
    jsonrpc_return_success(request, "{}", nullptr);
}

// Implements the "get_gpio" RPC.
// Takes one parameter:
//    "pin" is the numerical value of the pin to get the state of.
// Returns success or failure.
static void GetGpio(struct jsonrpc_request *request) {
    double pin_double;
    int pin;

    const char *pin_param_pattern = "$[0].pin";

    int find_result;
    find_result = mjson_find(request->params, request->params_len, pin_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, pin_param_pattern, &pin_double);
        pin = static_cast<int>(pin_double);
    } else {
        jsonrpc_return_error(request, -1, "'pin' missing", nullptr);
        return;
    }

    auto pin_gpio_values = j5_j6_gpio_pins.find(pin);
    if (pin_gpio_values == j5_j6_gpio_pins.end()) {
        jsonrpc_return_error(request, -1, "invalid pin", nullptr);
        return;
    }

    int pin_value = GPIO_PinRead(pin_gpio_values->second.first, pin_gpio_values->second.second);
    jsonrpc_return_success(request, "{%Q:%d}", "value", pin_value);
}

// Implements the "capture_test_pattern" RPC.
// Configures the sensor to test pattern mode, and captures via trigger.
// Returns success if the test pattern has the expected data, failure otherwise.
static void CaptureTestPattern(struct jsonrpc_request *request) {
    valiant::CameraTask::GetSingleton()->SetPower(true);
    valiant::CameraTask::GetSingleton()->Enable(valiant::camera::Mode::TRIGGER);
    valiant::CameraTask::GetSingleton()->SetTestPattern(
            valiant::camera::TestPattern::WALKING_ONES);

    valiant::CameraTask::GetSingleton()->Trigger();

    uint8_t* buffer = nullptr;
    int index = valiant::CameraTask::GetSingleton()->GetFrame(&buffer, true);
    uint8_t expected = 0;
    bool success = true;
    for (unsigned int i = 0; i < valiant::CameraTask::kWidth * valiant::CameraTask::kHeight; ++i) {
        if (buffer[i] != expected) {
            success = false;
            break;
        }
        if (expected == 0) {
            expected = 1;
        } else {
            expected = expected << 1;
        }
    }
    if (success) {
        jsonrpc_return_success(request, "{}");
    } else {
        jsonrpc_return_error(request, -1, "camera test pattern mismatch", nullptr);
    }
    valiant::CameraTask::GetSingleton()->ReturnFrame(index);
    valiant::CameraTask::GetSingleton()->SetPower(false);
}

// Implements the "capture_audio" RPC.
// Attempts to capture 1 second of audio.
// Returns success, with a parameter "data" containing the captured audio in base64 (or failure).
// The audio captured is 16-bit signed PCM @ 16000Hz.
static void CaptureAudio(struct jsonrpc_request *request) {
    struct AudioParams {
        SemaphoreHandle_t sema;
        int count;
        uint16_t* audio_data;
    };
    AudioParams params;
    std::vector<uint16_t> audio_data(16000, 0xA5A5);
    size_t audio_data_size = sizeof(uint16_t) * audio_data.size();
    params.sema = xSemaphoreCreateBinary();
    params.count = 0;
    params.audio_data = audio_data.data();
    valiant::AudioTask::GetSingleton()->SetCallback([](void *param) {
        AudioParams* params = reinterpret_cast<AudioParams*>(param);
        // If this is the second time we hit the callback, return nothing for the buffer
        // to terminate recording.
        // Otherwise, we feed the original buffer back in. This should give us a clean recording
        // of one second, without any sort of pops or glitches that happen when the mic starts.
        if (params->count == 1) {
            BaseType_t reschedule = pdFALSE;
            xSemaphoreGiveFromISR(params->sema, &reschedule);
            portYIELD_FROM_ISR(reschedule);
            return reinterpret_cast<uint32_t*>(0);
        } else {
            params->count++;
            return reinterpret_cast<uint32_t*>(params->audio_data);
        }
    }, &params);

    valiant::AudioTask::GetSingleton()->SetPower(true);
    valiant::AudioTask::GetSingleton()->SetBuffer(reinterpret_cast<uint32_t*>(audio_data.data()), audio_data_size);
    valiant::AudioTask::GetSingleton()->Enable();
    BaseType_t ret = xSemaphoreTake(params.sema, pdMS_TO_TICKS(3000));
    valiant::AudioTask::GetSingleton()->Disable();
    valiant::AudioTask::GetSingleton()->SetPower(false);
    vSemaphoreDelete(params.sema);

    if (ret != pdTRUE) {
        jsonrpc_return_error(request, -1, "semaphore timed out", nullptr);
    } else {
        size_t encoded_length = 0;
        mbedtls_base64_encode(nullptr, 0, &encoded_length, reinterpret_cast<uint8_t*>(audio_data.data()), audio_data_size);
        std::vector<uint8_t> encoded_data(encoded_length);
        mbedtls_base64_encode(encoded_data.data(), encoded_length, &encoded_length, reinterpret_cast<uint8_t*>(audio_data.data()), audio_data_size);
        jsonrpc_return_success(request, "{%Q:%.*Q}", "data", encoded_length, encoded_data.data());
    }
}

// Implements the "set_tpu_power_state" RPC.
// Takes one parameter, "enable" -- a boolean indicating the state to set.
// Returns success or failure.
static void SetTPUPowerState(struct jsonrpc_request *request) {
    int enable;
    const char *enable_param_pattern = "$[0].enable";

    int find_result = mjson_find(request->params, request->params_len, enable_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_TRUE || find_result == MJSON_TOK_FALSE) {
        mjson_get_bool(request->params, request->params_len, enable_param_pattern, &enable);
    } else {
        jsonrpc_return_error(request, -1, "'enable' missing or invalid", nullptr);
        return;
    }

    valiant::EdgeTpuTask::GetSingleton()->SetPower(enable);
    jsonrpc_return_success(request, "{}");
}

// Implements the "run_testconv1" RPC.
// Runs the simple "testconv1" model using the TPU.
// NOTE: The TPU power must be enabled for this RPC to succeed.
static void RunTestConv1(struct jsonrpc_request *request) {
    valiant::EdgeTpuManager::GetSingleton()->OpenDevice();
    if (!valiant::testconv1::setup()) {
        jsonrpc_return_error(request, -1, "testconv1 setup failed", nullptr);
        return;
    }
    if (!valiant::testconv1::loop()) {
        jsonrpc_return_error(request, -1, "testconv1 loop failed", nullptr);
        return;
    }
    jsonrpc_return_success(request, "{}");
}

static void GetTPUChipIds(struct jsonrpc_request *request) {
    jsonrpc_return_error(request, -1, "get_tpu_chip_ids not implemented", nullptr);
}

static void CheckTPUAlarm(struct jsonrpc_request *request) {
    jsonrpc_return_error(request, -1, "check_tpu_alarm not implemented", nullptr);
}

// Implements the "set_dac_value" RPC.
// Takes one parameter, "counts". This represents the number of DAC counts to set, from 0-4095.
// Returns success or failure.
static void SetDACValue(struct jsonrpc_request *request) {
    double counts_double;
    const char *counts_param_pattern = "$[0].counts";

    int find_result = mjson_find(request->params, request->params_len, counts_param_pattern, nullptr, nullptr);
    if (find_result == MJSON_TOK_NUMBER) {
        mjson_get_number(request->params, request->params_len, counts_param_pattern, &counts_double);
    } else {
        jsonrpc_return_error(request, -1, "'counts' missing or invalid", nullptr);
        return;
    }

    int counts = static_cast<int>(counts_double);
    if (counts > 4095 || counts < 0) {
        jsonrpc_return_error(request, -1, "'counts' out of range (0-4095)", nullptr);
        return;
    }

    valiant::analog::Init(valiant::analog::Device::DAC1);
    if (counts) {
        valiant::analog::EnableDAC(true);
        valiant::analog::WriteDAC(counts);
    } else {
        valiant::analog::EnableDAC(false);
    }

    jsonrpc_return_success(request, "{}");
}

// Implements "test_sdram_pattern" RPC.
// Allocates memory from SDRAM, writes and verifies a test pattern.
// Returns success or failure.
static void TestSDRamPattern(struct jsonrpc_request *request) {
    size_t sdram_area_size = 1024 * 1024; // 1 MB
    std::unique_ptr<uint8_t[]> sdram_area(new uint8_t[sdram_area_size]);
    if (sdram_area.get() < reinterpret_cast<void*>(0x80000000U)) {
        jsonrpc_return_error(request, -1, "test_sdram_pattern memory not in sdram", nullptr);
        return;
    }

    for (size_t i = 0; i < sdram_area_size; ++i) {
        sdram_area[i] = i % UCHAR_MAX;
    }

    for (size_t i = 0; i < sdram_area_size; ++i) {
        uint8_t val = sdram_area[i];
        if (val != i % UCHAR_MAX) {
            jsonrpc_return_error(request, -1, "test_sdram_pattern failed", "{%Q:%x}", "location", &sdram_area[i]);
            return;
        }
    }

    jsonrpc_return_success(request, "{}");
}

// Implements the "write_file" RPC.
// Takes two parameters:
//    "filename": Path in the filesystem to write to.
//    "data": base64-encoded data to decode and write into the file.
// Returns success or failure.
static void WriteFile(struct jsonrpc_request *request) {
    const char *filename_param_pattern = "$[0].filename";
    const char *data_param_pattern = "$[0].data";
    ssize_t size = 0;
    int find_result;

    std::vector<char> filename, data;
    find_result = mjson_find(request->params, request->params_len, filename_param_pattern, nullptr, &size);
    if (find_result == MJSON_TOK_STRING) {
        filename.resize(size, 0);
        mjson_get_string(request->params, request->params_len, filename_param_pattern, filename.data(), size);
    } else {
        jsonrpc_return_error(request, -1, "'filename' missing or invalid", nullptr);
        return;
    }
    find_result = mjson_find(request->params, request->params_len, data_param_pattern, nullptr, &size);
    if (find_result == MJSON_TOK_STRING) {
        data.resize(size, 0);
        mjson_get_string(request->params, request->params_len, data_param_pattern, data.data(), size);
    } else {
        jsonrpc_return_error(request, -1, "'data' missing or invalid", nullptr);
        return;
    }

    lfs_file_t handle;
    bool ret = valiant::filesystem::Open(&handle, filename.data(), true);
    if (!ret) {
        jsonrpc_return_error(request, -1, "failed to open file", nullptr);
        return;
    }


    // The data from mjson is a null-terminated string: use strlen for getting the size.
    unsigned int bytes_to_decode = strlen(data.data());
    size_t decoded_length = 0;
    mbedtls_base64_decode(nullptr, 0, &decoded_length, reinterpret_cast<unsigned char*>(data.data()), bytes_to_decode);
    std::vector<uint8_t> decoded_data(decoded_length);
    mbedtls_base64_decode(decoded_data.data(), decoded_length, &decoded_length, reinterpret_cast<unsigned char*>(data.data()), bytes_to_decode);

    int bytes_written = valiant::filesystem::Write(&handle, decoded_data.data(), decoded_length);
    if (static_cast<unsigned int>(bytes_written) != decoded_length) {
        jsonrpc_return_error(request, -1, "did not write all bytes", nullptr);
        valiant::filesystem::Close(&handle);
        return;
    }

    ret = valiant::filesystem::Close(&handle);
    if (!ret) {
        jsonrpc_return_error(request, -1, "failed to close file", nullptr);
    } else {
        jsonrpc_return_success(request, "{}");
    }
}

// Implements the "read_file" RPC.
// Takes one parameter, "filename".
// Base64-encodes and returns the data in the file, if it exists.
static void ReadFile(struct jsonrpc_request *request) {
    const char *filename_param_pattern = "$[0].filename";
    ssize_t size = 0;
    int find_result;

    std::vector<char> filename;
    find_result = mjson_find(request->params, request->params_len, filename_param_pattern, nullptr, &size);
    if (find_result == MJSON_TOK_STRING) {
        filename.resize(size, 0);
        mjson_get_string(request->params, request->params_len, filename_param_pattern, filename.data(), size);
    } else {
        jsonrpc_return_error(request, -1, "'filename' missing or invalid", nullptr);
        return;
    }

    lfs_file_t handle;
    bool ret = valiant::filesystem::Open(&handle, filename.data());

    if (!ret) {
        jsonrpc_return_error(request, -1, "failed to open file", nullptr);
        return;
    }

    lfs_soff_t file_size = valiant::filesystem::Size(&handle);
    if (file_size <= 0) {
        jsonrpc_return_error(request, -1, "bad file size", nullptr);
        return;
    }

    std::vector<uint8_t> data(file_size);
    int bytes_read = valiant::filesystem::Read(&handle, data.data(), file_size);
    if (bytes_read < file_size) {
        jsonrpc_return_error(request, -1, "failed to read all bytes", nullptr);
        valiant::filesystem::Close(&handle);
        return;
    }

    ret = valiant::filesystem::Close(&handle);
    if (!ret) {
        jsonrpc_return_error(request, -1, "failed to close file", nullptr);
    } else {
        size_t encoded_length = 0;
        mbedtls_base64_encode(nullptr, 0, &encoded_length, data.data(), data.size());
        std::vector<uint8_t> encoded_data(encoded_length);
        mbedtls_base64_encode(encoded_data.data(), encoded_length, &encoded_length, data.data(), data.size());
        jsonrpc_return_success(request, "{%Q:%.*Q}", "data", encoded_length, encoded_data.data());
    }
}

extern "C" void app_main(void *param) {
    InitializeLoopbackMappings();
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

    rpc_server.RegisterRPC("get_serial_number", GetSerialNumber);
    rpc_server.RegisterRPC("set_pmic_rail_state", SetPmicRailState);
    rpc_server.RegisterRPC("set_led_state", SetLedState);
    // TODO(atv): Special handling for the pair with DAC_OUT
    rpc_server.RegisterRPC("set_pin_pair_to_gpio", SetPinPairToGpio);
    rpc_server.RegisterRPC("set_gpio", SetGpio);
    rpc_server.RegisterRPC("get_gpio", GetGpio);
    rpc_server.RegisterRPC("capture_test_pattern", CaptureTestPattern);
    rpc_server.RegisterRPC("capture_audio", CaptureAudio);
    rpc_server.RegisterRPC("set_tpu_power_state", SetTPUPowerState);
    rpc_server.RegisterRPC("run_testconv1", RunTestConv1);
    rpc_server.RegisterRPC("get_tpu_chip_ids", GetTPUChipIds);
    rpc_server.RegisterRPC("check_tpu_alarm", CheckTPUAlarm);
    rpc_server.RegisterRPC("set_dac_value", SetDACValue);
    rpc_server.RegisterRPC("test_sdram_pattern", TestSDRamPattern);
    rpc_server.RegisterRPC("write_file", WriteFile);
    rpc_server.RegisterRPC("read_file", ReadFile);

    vTaskSuspend(NULL);
}

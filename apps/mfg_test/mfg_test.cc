// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <array>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "apps/mfg_test/mfg_test_iperf.h"
#include "libs/a71ch/a71ch.h"
#include "libs/base/analog.h"
#include "libs/base/ethernet.h"
#include "libs/base/filesystem.h"
#include "libs/base/led.h"
#include "libs/base/utils.h"
#include "libs/pmic/pmic.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "libs/testlib/test_lib.h"
#include "libs/tpu/edgetpu_task.h"
#include "third_party/a71ch-crypto-support/hostlib/hostLib/inc/a71ch_api.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_iomuxc.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_ocotp.h"

namespace {
using coralmicro::JsonRpcGetBase64Param;
using coralmicro::JsonRpcGetBooleanParam;
using coralmicro::JsonRpcGetIntegerParam;
using coralmicro::JsonRpcGetStringParam;

// In the below maps, data about pins to be tested via the loopback fixture
// is provided. Pins on J5 are numbered from 1-100, and pins on J6 are numbered
// from 101-200.

// Map from pin-to-pin: Used to validate that the provided
// pair will be connected via loopback fixture.
std::map<int, int> j5_j6_loopback_mapping;

// IOMUXC entries for each pin to be tested.
std::map<int, std::array<uint32_t, 5>> j5_j6_iomuxc;

// GPIO module / pin number for each pin to be tested.
std::map<int, std::pair<GPIO_Type*, int>> j5_j6_gpio_pins;

constexpr int kDacPin = 95;

void InitializeLoopbackMappings() {
  j5_j6_loopback_mapping = {
      // clang-format off
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
        {44, 41},
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
        {95, 14},
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
      // clang-format on
  };
  j5_j6_iomuxc = {
      {1, {IOMUXC_GPIO_AD_26_GPIO9_IO25}},
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
      {88, {IOMUXC_GPIO_DISP_B2_06_GPIO11_IO07}},
      {89, {IOMUXC_GPIO_AD_24_GPIO9_IO23}},
      {91, {IOMUXC_GPIO_AD_25_GPIO9_IO24}},
      {92, {IOMUXC_GPIO_SD_B1_01_GPIO10_IO04}},
      {93, {IOMUXC_GPIO_AD_34_GPIO10_IO01}},
      {94, {IOMUXC_GPIO_SD_B1_03_GPIO10_IO06}},
      {95, {}},
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
      {166, {IOMUXC_GPIO_DISP_B1_04_GPIO10_IO25}},
      {168, {IOMUXC_GPIO_DISP_B1_03_GPIO10_IO24}},
      {169, {IOMUXC_GPIO_AD_28_GPIO9_IO27}},
      {170, {IOMUXC_GPIO_DISP_B1_02_GPIO10_IO23}},
      {171, {IOMUXC_GPIO_AD_29_GPIO9_IO28}},
      {172, {IOMUXC_GPIO_DISP_B1_05_GPIO10_IO26}},
      {173, {IOMUXC_GPIO_AD_30_GPIO9_IO29}},
      {175, {IOMUXC_GPIO_AD_31_GPIO9_IO30}},
      {176, {IOMUXC_GPIO_DISP_B1_00_GPIO10_IO21}},
      {178, {IOMUXC_GPIO_DISP_B1_01_GPIO10_IO22}},
  };

  j5_j6_gpio_pins = {
      // clang-format off
        {1, {GPIO9, 25}},
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
        {88, {GPIO11, 7}},
        {89, {GPIO9, 23}},
        {91, {GPIO9, 24}},
        {92, {GPIO10, 4}},
        {93, {GPIO10, 1}},
        {94, {GPIO10, 6}},
        {95, {}},
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
        {166, {GPIO10, 25}},
        {168, {GPIO10, 24}},
        {169, {GPIO9, 27}},
        {170, {GPIO10, 23}},
        {171, {GPIO9, 28}},
        {172, {GPIO10, 26}},
        {173, {GPIO9, 29}},
        {175, {GPIO9, 30}},
        {176, {GPIO10, 21}},
        {178, {GPIO10, 22}},
      // clang-format on
  };
}
// Implementation of "set_pmic_rail_state" RPC.
// Takes two parameters:
//    "rail" is an enumerated value indicating the rail to change.
//    "enable" is a boolean state to set the rail to.
// Returns success or failure to set the requested state.
void SetPmicRailState(struct jsonrpc_request* request) {
  int rail;
  if (!JsonRpcGetIntegerParam(request, "rail", &rail)) return;

  bool enable;
  if (!JsonRpcGetBooleanParam(request, "enable", &enable)) return;

  coralmicro::PmicTask::GetSingleton()->SetRailState(
      static_cast<coralmicro::PmicRail>(rail), enable);
  jsonrpc_return_success(request, "{}");
}

// Implementation of "set_led_state" RPC.
// Takes two parameters:
//    "led" is an enumerated value indicating the LED to change.
//    "enable" is a boolean state to set the rail to.
// Returns success or failure to set the requested state.
// NOTE: The TPU LED requires that the TPU power is enabled.
void SetLedState(struct jsonrpc_request* request) {
  int led;
  if (!JsonRpcGetIntegerParam(request, "led", &led)) return;

  bool enable;
  if (!JsonRpcGetBooleanParam(request, "enable", &enable)) return;

  enum LEDs {
    kStatus = 0,
    kUser = 1,
    kTpu = 2,
  };
  switch (led) {
    case kStatus:
      coralmicro::LedSet(coralmicro::Led::kStatus, enable);
      break;
    case kUser:
      coralmicro::LedSet(coralmicro::Led::kUser, enable);
      break;
    case kTpu:
      if (!coralmicro::EdgeTpuTask::GetSingleton()->GetPower()) {
        jsonrpc_return_error(request, -1, "TPU power is not enabled", nullptr);
        return;
      }
      coralmicro::LedSet(coralmicro::Led::kTpu, enable);
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
void SetPinPairToGpio(struct jsonrpc_request* request) {
  int output_pin, input_pin;

  if (!JsonRpcGetIntegerParam(request, "output_pin", &output_pin)) return;
  if (!JsonRpcGetIntegerParam(request, "input_pin", &input_pin)) return;

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
  if (pin_pair_a->first != pin_pair_b->second ||
      pin_pair_b->first != pin_pair_a->second) {
    jsonrpc_return_error(
        request, -1, "'output_pin and input_pin are not a pair'",
        "{%Q:%d, %Q:%d}", "output_pin", output_pin, "input_pin", input_pin);
    return;
  }

  auto output_pin_mux = j5_j6_iomuxc.find(output_pin);
  auto input_pin_mux = j5_j6_iomuxc.find(input_pin);
  if (output_pin_mux == j5_j6_iomuxc.end()) {
    jsonrpc_return_error(request, -1, "'output_pin' mux settings not found",
                         nullptr);
    return;
  }
  if (input_pin_mux == j5_j6_iomuxc.end()) {
    jsonrpc_return_error(request, -1, "'input_pin' mux settings not found",
                         nullptr);
    return;
  }
  auto output_pin_gpio_values = j5_j6_gpio_pins.find(output_pin);
  if (output_pin_gpio_values == j5_j6_gpio_pins.end()) {
    jsonrpc_return_error(request, -1, "'output_pin' gpio settings not found",
                         nullptr);
    return;
  }
  auto input_pin_gpio_values = j5_j6_gpio_pins.find(input_pin);
  if (input_pin_gpio_values == j5_j6_gpio_pins.end()) {
    jsonrpc_return_error(request, -1, "'input_pin' gpio settings not found",
                         nullptr);
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
  const uint32_t kInputBufferOn = 1U;
  const uint32_t kDisablePulls = 0U;
  if (output_pin != kDacPin) {
    GPIO_PinInit(output_pin_gpio_values->second.first,
                 output_pin_gpio_values->second.second, &pin_config_output);
    IOMUXC_SetPinMux(output_pin_mux->second[0], output_pin_mux->second[1],
                     output_pin_mux->second[2], output_pin_mux->second[3],
                     output_pin_mux->second[4], kInputBufferOn);
    IOMUXC_SetPinConfig(output_pin_mux->second[0], output_pin_mux->second[1],
                        output_pin_mux->second[2], output_pin_mux->second[3],
                        output_pin_mux->second[4], kDisablePulls);
  }

  if (input_pin != kDacPin) {
    GPIO_PinInit(input_pin_gpio_values->second.first,
                 input_pin_gpio_values->second.second, &pin_config_input);
    IOMUXC_SetPinMux(input_pin_mux->second[0], input_pin_mux->second[1],
                     input_pin_mux->second[2], input_pin_mux->second[3],
                     input_pin_mux->second[4], kInputBufferOn);
    IOMUXC_SetPinConfig(input_pin_mux->second[0], input_pin_mux->second[1],
                        input_pin_mux->second[2], input_pin_mux->second[3],
                        input_pin_mux->second[4], kDisablePulls);
  }

  jsonrpc_return_success(request, "{}");
}

// Implements the "set_gpio" RPC.
// Takes two parameters:
//    "pin" is the numerical value of the pin to set a state for.
//    "enable" is whether to drive the pin high or low.
// Returns success or failure.
void SetGpio(struct jsonrpc_request* request) {
  int pin;
  if (!JsonRpcGetIntegerParam(request, "pin", &pin)) return;

  bool enable;
  if (!JsonRpcGetBooleanParam(request, "enable", &enable)) return;

  auto pin_gpio_values = j5_j6_gpio_pins.find(pin);
  if (pin_gpio_values == j5_j6_gpio_pins.end()) {
    jsonrpc_return_error(request, -1, "invalid pin", nullptr);
    return;
  }

  if (pin != kDacPin) {
    GPIO_PinWrite(pin_gpio_values->second.first, pin_gpio_values->second.second,
                  enable);
  } else {
    coralmicro::DacWrite(enable ? 4095 : 1);
    coralmicro::DacEnable(true);
  }
  jsonrpc_return_success(request, "{}", nullptr);
}

// Implements the "get_gpio" RPC.
// Takes one parameter:
//    "pin" is the numerical value of the pin to get the state of.
// Returns success or failure.
void GetGpio(struct jsonrpc_request* request) {
  int pin;
  if (!JsonRpcGetIntegerParam(request, "pin", &pin)) return;

  auto pin_gpio_values = j5_j6_gpio_pins.find(pin);
  if (pin_gpio_values == j5_j6_gpio_pins.end()) {
    jsonrpc_return_error(request, -1, "invalid pin", nullptr);
    return;
  }

  int pin_value = GPIO_PinRead(pin_gpio_values->second.first,
                               pin_gpio_values->second.second);
  jsonrpc_return_success(request, "{%Q:%d}", "value", pin_value);
}

void GetTPUChipIds(struct jsonrpc_request* request) {
  jsonrpc_return_error(request, -1, "get_tpu_chip_ids not implemented",
                       nullptr);
}

void CheckTPUAlarm(struct jsonrpc_request* request) {
  jsonrpc_return_error(request, -1, "check_tpu_alarm not implemented", nullptr);
}

// Implements the "set_dac_value" RPC.
// Takes one parameter, "counts". This represents the number of DAC counts to
// set, from 0-4095. Returns success or failure.
void SetDACValue(struct jsonrpc_request* request) {
  int counts;
  if (!JsonRpcGetIntegerParam(request, "counts", &counts)) return;

  if (counts > 4095 || counts < 0) {
    jsonrpc_return_error(request, -1, "'counts' out of range (0-4095)",
                         nullptr);
    return;
  }

  coralmicro::DacWrite(counts);
  coralmicro::DacEnable(!!counts);
  jsonrpc_return_success(request, "{}");
}

// Implements "test_sdram_pattern" RPC.
// Allocates memory from SDRAM, writes and verifies a test pattern.
// Returns success or failure.
void TestSDRamPattern(struct jsonrpc_request* request) {
  size_t sdram_area_size = 1024 * 1024;  // 1 MB
  auto sdram_area = std::make_unique<uint8_t[]>(sdram_area_size);
  if (sdram_area.get() < reinterpret_cast<void*>(0x80000000U)) {
    jsonrpc_return_error(request, -1, "test_sdram_pattern memory not in sdram",
                         nullptr);
    return;
  }

  for (size_t i = 0; i < sdram_area_size; ++i) {
    sdram_area[i] = i % UCHAR_MAX;
  }

  for (size_t i = 0; i < sdram_area_size; ++i) {
    uint8_t val = sdram_area[i];
    if (val != i % UCHAR_MAX) {
      jsonrpc_return_error(request, -1, "test_sdram_pattern failed", "{%Q:%x}",
                           "location", &sdram_area[i]);
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
void WriteFile(struct jsonrpc_request* request) {
  std::string filename;
  if (!JsonRpcGetStringParam(request, "filename", &filename)) return;

  std::vector<uint8_t> data;
  if (!JsonRpcGetBase64Param(request, "data", &data)) return;

  if (!coralmicro::LfsWriteFile(filename.c_str(), data.data(), data.size())) {
    jsonrpc_return_error(request, -1, "failed to write file", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{}");
}

// Implements the "read_file" RPC.
// Takes one parameter, "filename".
// Base64-encodes and returns the data in the file, if it exists.
void ReadFile(struct jsonrpc_request* request) {
  std::string filename;
  if (!JsonRpcGetStringParam(request, "filename", &filename)) return;

  std::vector<uint8_t> data;
  if (!coralmicro::LfsReadFile(filename.c_str(), &data)) {
    jsonrpc_return_error(request, -1, "failed to read file", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{%Q: %V}", "data", data.size(), data.data());
}

void CheckA71CH(struct jsonrpc_request* request) {
  static bool a71ch_inited = false;
  if (!a71ch_inited) {
    bool success = coralmicro::A71ChInit();
    if (!success) {
      jsonrpc_return_error(request, -1, "failed to init a71ch", nullptr);
      return;
    }
    a71ch_inited = true;
  }

  uint8_t uid[A71CH_MODULE_UNIQUE_ID_LEN];
  uint16_t uidLen = A71CH_MODULE_UNIQUE_ID_LEN;
  uint16_t ret = A71_GetUniqueID(uid, &uidLen);

  if (ret == SMCOM_OK) {
    jsonrpc_return_success(request, "{}");
  } else {
    jsonrpc_return_error(request, -1, "failed to retrieve a71ch id", nullptr);
  }
}

void FuseMACAddress(struct jsonrpc_request* request) {
  std::string address;
  if (!JsonRpcGetStringParam(request, "address", &address)) return;

  unsigned int a, b, c, d, e, f;
  int tokens = sscanf(address.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X", &a, &b,
                      &c, &d, &e, &f);
  if (tokens != 6) {
    jsonrpc_return_error(request, -1, "could not get six octets from 'address'",
                         nullptr);
    return;
  }

  OCOTP_Init(OCOTP, 0);

  uint32_t mac_address_hi, mac_address_lo;
  status_t status = OCOTP_ReadFuseShadowRegisterExt(
      OCOTP, FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_HI), &mac_address_hi, 1);
  if (status != kStatus_Success) {
    jsonrpc_return_error(request, -1,
                         "failed to read MAC address fuse register", nullptr);
    return;
  }

  // Preserve the reserved bits at the top of the high register.
  mac_address_hi &= 0xFFFF0000;
  mac_address_hi |= (a << 8) | (b);
  mac_address_lo = (c << 24) | (d << 16) | (e << 8) | f;

  status = OCOTP_WriteFuseShadowRegister(
      OCOTP, FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_HI), mac_address_hi);
  if (status != kStatus_Success) {
    jsonrpc_return_error(request, -1,
                         "failed to write MAC address fuse register", nullptr);
    return;
  }
  status = OCOTP_WriteFuseShadowRegister(
      OCOTP, FUSE_ADDRESS_TO_OCOTP_INDEX(MAC1_ADDR_LO), mac_address_lo);
  if (status != kStatus_Success) {
    jsonrpc_return_error(request, -1,
                         "failed to write MAC address fuse register", nullptr);
    return;
  }

  jsonrpc_return_success(request, "{}");
}

void ReadMACAddress(struct jsonrpc_request* request) {
  auto mac = coralmicro::EthernetGetMacAddress();
  char mac_str[255];
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);
  jsonrpc_return_success(request, "{%Q:%Q}", "address", mac_str);
}
}  // namespace

extern "C" void app_main(void* param) {
  InitializeLoopbackMappings();
  coralmicro::DacInit();

  jsonrpc_init(nullptr, nullptr);
  jsonrpc_export(coralmicro::testlib::kMethodGetSerialNumber,
                 coralmicro::testlib::GetSerialNumber);
  jsonrpc_export(coralmicro::testlib::kMethodGetSerialNumber,
                 coralmicro::testlib::GetSerialNumber);
  jsonrpc_export("set_pmic_rail_state", SetPmicRailState);
  jsonrpc_export("set_led_state", SetLedState);
  // TODO(atv): Special handling for the pair with DAC_OUT
  jsonrpc_export("set_pin_pair_to_gpio", SetPinPairToGpio);
  jsonrpc_export("set_gpio", SetGpio);
  jsonrpc_export("get_gpio", GetGpio);
  jsonrpc_export(coralmicro::testlib::kMethodCaptureTestPattern,
                 coralmicro::testlib::CaptureTestPattern);
  jsonrpc_export(coralmicro::testlib::kMethodCaptureAudio,
                 coralmicro::testlib::CaptureAudio);
  jsonrpc_export(coralmicro::testlib::kMethodSetTPUPowerState,
                 coralmicro::testlib::SetTPUPowerState);
  jsonrpc_export(coralmicro::testlib::kMethodRunTestConv1,
                 coralmicro::testlib::RunTestConv1);
  jsonrpc_export("get_tpu_chip_ids", GetTPUChipIds);
  jsonrpc_export("check_tpu_alarm", CheckTPUAlarm);
  jsonrpc_export("set_dac_value", SetDACValue);
  jsonrpc_export("test_sdram_pattern", TestSDRamPattern);
  jsonrpc_export("write_file", WriteFile);
  jsonrpc_export("read_file", ReadFile);
  jsonrpc_export("check_a71ch", CheckA71CH);
  jsonrpc_export("fuse_mac_address", FuseMACAddress);
  jsonrpc_export("read_mac_address", ReadMACAddress);
  IperfInit();
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
  vTaskSuspend(nullptr);
}

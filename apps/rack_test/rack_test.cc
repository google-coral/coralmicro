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

#include "apps/rack_test/rack_test_ipc.h"
#include "libs/base/ipc_m7.h"
#include "libs/base/utils.h"
#include "libs/camera/camera.h"
#include "libs/rpc/rpc_http_server.h"
#include "libs/rpc/rpc_utils.h"
#include "libs/testlib/test_lib.h"
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/freertos_kernel/include/task.h"
#include "third_party/modified/coremark/core_portme.h"

#if defined TEST_WIFI
#include "libs/base/wifi.h"
#endif
#if defined TEST_BLE
#include "libs/nxp/rt1176-sdk/edgefast_bluetooth/edgefast_bluetooth.h"
#endif

namespace {
constexpr char kMethodM4XOR[] = "m4_xor";
constexpr char kMethodM4CoreMark[] = "m4_coremark";
constexpr char kMethodM7CoreMark[] = "m7_coremark";
constexpr char kMethodGetFrame[] = "get_frame";

std::vector<uint8_t> camera_rgb;

void HandleAppMessage(const uint8_t data[coralmicro::kIpcMessageBufferDataSize],
                      TaskHandle_t rpc_task_handle) {
  const auto* app_message = reinterpret_cast<const RackTestAppMessage*>(data);
  switch (app_message->message_type) {
    case RackTestAppMessageType::kXor: {
      xTaskNotify(rpc_task_handle, app_message->message.xor_value,
                  eSetValueWithOverwrite);
      break;
    }
    case RackTestAppMessageType::kCoreMark: {
      xTaskNotify(rpc_task_handle, 0, eSetValueWithOverwrite);
      break;
    }
    default:
      printf("Unknown message type\r\n");
  }
}

void M4XOR(struct jsonrpc_request* request) {
  std::string value_string;
  if (!coralmicro::JsonRpcGetStringParam(request, "value", &value_string))
    return;

  if (!coralmicro::IpcM7::GetSingleton()->M4IsAlive(1000 /*ms*/)) {
    jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
    return;
  }

  auto value =
      reinterpret_cast<uint32_t>(strtoul(value_string.c_str(), nullptr, 10));
  coralmicro::IpcMessage msg{};
  msg.type = coralmicro::IpcMessageType::kApp;
  auto* app_message = reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
  app_message->message_type = RackTestAppMessageType::kXor;
  app_message->message.xor_value = value;
  coralmicro::IpcM7::GetSingleton()->SendMessage(msg);

  // hang out here and wait for an event.
  uint32_t xor_value;
  if (xTaskNotifyWait(0, 0, &xor_value, pdMS_TO_TICKS(1000)) != pdTRUE) {
    jsonrpc_return_error(request, -1, "Timed out waiting for response from M4",
                         nullptr);
    return;
  }

  jsonrpc_return_success(request, "{%Q:%lu}", "value", xor_value);
}

void M4CoreMark(struct jsonrpc_request* request) {
  auto* ipc = coralmicro::IpcM7::GetSingleton();
  if (!ipc->M4IsAlive(1000 /*ms*/)) {
    jsonrpc_return_error(request, -1, "M4 has not been started", nullptr);
    return;
  }

  char coremark_buffer[MAX_COREMARK_BUFFER];
  coralmicro::IpcMessage msg{};
  msg.type = coralmicro::IpcMessageType::kApp;
  auto* app_message = reinterpret_cast<RackTestAppMessage*>(&msg.message.data);
  app_message->message_type = RackTestAppMessageType::kCoreMark;
  app_message->message.buffer_ptr = coremark_buffer;
  coralmicro::IpcM7::GetSingleton()->SendMessage(msg);

  if (xTaskNotifyWait(0, 0, nullptr, pdMS_TO_TICKS(30000)) != pdTRUE) {
    jsonrpc_return_error(request, -1, "Timed out waiting for response from M4",
                         nullptr);
    return;
  }

  jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results",
                         coremark_buffer);
}

void M7CoreMark(struct jsonrpc_request* request) {
  char coremark_buffer[MAX_COREMARK_BUFFER];
  RunCoreMark(coremark_buffer);
  jsonrpc_return_success(request, "{%Q:%Q}", "coremark_results",
                         coremark_buffer);
}

void GetFrame(struct jsonrpc_request* request) {
  int rpc_width, rpc_height;
  std::string rpc_format;
  bool rpc_width_valid =
      coralmicro::JsonRpcGetIntegerParam(request, "width", &rpc_width);
  bool rpc_height_valid =
      coralmicro::JsonRpcGetIntegerParam(request, "height", &rpc_height);
  bool rpc_format_valid =
      coralmicro::JsonRpcGetStringParam(request, "format", &rpc_format);

  int width = rpc_width_valid ? rpc_width : coralmicro::CameraTask::kWidth;
  int height = rpc_height_valid ? rpc_height : coralmicro::CameraTask::kHeight;
  auto format = coralmicro::CameraFormat::kRgb;

  if (rpc_format_valid) {
    constexpr char kFormatRGB[] = "RGB";
    constexpr char kFormatGrayscale[] = "L";
    if (memcmp(rpc_format.c_str(), kFormatRGB,
               std::min(rpc_format.length(), strlen(kFormatRGB))) == 0) {
      format = coralmicro::CameraFormat::kRgb;
    }
    if (memcmp(rpc_format.c_str(), kFormatGrayscale,
               std::min(rpc_format.length(), strlen(kFormatGrayscale))) == 0) {
      format = coralmicro::CameraFormat::kY8;
    }
  }

  camera_rgb.resize(width * height * coralmicro::CameraFormatBpp(format));

  coralmicro::CameraTask::GetSingleton()->SetPower(true);
  auto pattern = coralmicro::CameraTestPattern::kColorBar;
  coralmicro::CameraTask::GetSingleton()->SetTestPattern(pattern);
  coralmicro::CameraTask::GetSingleton()->Enable(
      coralmicro::CameraMode::kStreaming);

  coralmicro::CameraFrameFormat fmt_rgb{};
  fmt_rgb.fmt = format;
  fmt_rgb.filter = coralmicro::CameraFilterMethod::kBilinear;
  fmt_rgb.width = width;
  fmt_rgb.height = height;
  fmt_rgb.preserve_ratio = false;
  fmt_rgb.buffer = camera_rgb.data();

  bool success = coralmicro::CameraTask::GetSingleton()->GetFrame({fmt_rgb});
  coralmicro::CameraTask::GetSingleton()->SetPower(false);

  if (success)
    jsonrpc_return_success(request, "{}");
  else
    jsonrpc_return_error(request, -1, "Call to GetFrame returned false.",
                         nullptr);
}

coralmicro::HttpServer::Content UriHandler(const char* name) {
  if (std::strcmp("/camera.rgb", name) == 0)
    return coralmicro::HttpServer::Content{std::move(camera_rgb)};
  return {};
}
}  // namespace

extern "C" void app_main(void* param) {
  coralmicro::IpcM7::GetSingleton()->RegisterAppMessageHandler(
      [handle = xTaskGetHandle(TCPIP_THREAD_NAME)](
          const uint8_t data[coralmicro::kIpcMessageBufferDataSize]) {
        HandleAppMessage(data, handle);
      });
  jsonrpc_init(nullptr, nullptr);
#if defined(TEST_WIFI)
  if (!coralmicro::WiFiTurnOn(/*default_iface=*/false)) {
    printf("Wi-Fi failed to come up (is the Wi-Fi board attached?)\r\n");
    vTaskSuspend(nullptr);
  }
  jsonrpc_export(coralmicro::testlib::kMethodWiFiSetAntenna,
                 coralmicro::testlib::WiFiSetAntenna);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiScan,
                 coralmicro::testlib::WiFiScan);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiConnect,
                 coralmicro::testlib::WiFiConnect);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiDisconnect,
                 coralmicro::testlib::WiFiDisconnect);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiGetIp,
                 coralmicro::testlib::WiFiGetIp);
  jsonrpc_export(coralmicro::testlib::kMethodWiFiGetStatus,
                 coralmicro::testlib::WiFiGetStatus);
#endif
  jsonrpc_export(coralmicro::testlib::kMethodGetSerialNumber,
                 coralmicro::testlib::GetSerialNumber);
  jsonrpc_export(coralmicro::testlib::kMethodRunTestConv1,
                 coralmicro::testlib::RunTestConv1);
  jsonrpc_export(coralmicro::testlib::kMethodSetTPUPowerState,
                 coralmicro::testlib::SetTPUPowerState);
  jsonrpc_export(coralmicro::testlib::kMethodPosenetStressRun,
                 coralmicro::testlib::PosenetStressRun);
  jsonrpc_export(coralmicro::testlib::kMethodBeginUploadResource,
                 coralmicro::testlib::BeginUploadResource);
  jsonrpc_export(coralmicro::testlib::kMethodUploadResourceChunk,
                 coralmicro::testlib::UploadResourceChunk);
  jsonrpc_export(coralmicro::testlib::kMethodDeleteResource,
                 coralmicro::testlib::DeleteResource);
  jsonrpc_export(coralmicro::testlib::kMethodFetchResource,
                 coralmicro::testlib::FetchResource);
  jsonrpc_export(coralmicro::testlib::kMethodRunClassificationModel,
                 coralmicro::testlib::RunClassificationModel);
  jsonrpc_export(coralmicro::testlib::kMethodRunDetectionModel,
                 coralmicro::testlib::RunDetectionModel);
  jsonrpc_export(coralmicro::testlib::kMethodRunSegmentationModel,
                 coralmicro::testlib::RunSegmentationModel);
  jsonrpc_export(coralmicro::testlib::kMethodStartM4,
                 coralmicro::testlib::StartM4);
  jsonrpc_export(coralmicro::testlib::kMethodGetTemperature,
                 coralmicro::testlib::GetTemperature);
  jsonrpc_export(kMethodM4XOR, M4XOR);
  jsonrpc_export(coralmicro::testlib::kMethodCaptureTestPattern,
                 coralmicro::testlib::CaptureTestPattern);
  jsonrpc_export(kMethodM4CoreMark, M4CoreMark);
  jsonrpc_export(kMethodM7CoreMark, M7CoreMark);
  jsonrpc_export(kMethodGetFrame, GetFrame);
  jsonrpc_export(coralmicro::testlib::kMethodCaptureAudio,
                 coralmicro::testlib::CaptureAudio);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoInit,
                 coralmicro::testlib::CryptoInit);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoGetUId,
                 coralmicro::testlib::CryptoGetUID);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoGetRandomBytes,
                 coralmicro::testlib::CryptoGetRandomBytes);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoGetEccSignature,
                 coralmicro::testlib::CryptoGetEccSignature);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoGetPubEccKey,
                 coralmicro::testlib::CryptoGetPublicEccKey);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoGetSha256,
                 coralmicro::testlib::CryptoGetSha256);
  jsonrpc_export(coralmicro::testlib::kMethodCryptoEccVerify,
                 coralmicro::testlib::CryptoEccVerify);
#if defined TEST_BLE
  InitEdgefastBluetooth(nullptr);
  jsonrpc_export(coralmicro::testlib::kMethodBleScan,
                 coralmicro::testlib::BleScan);
#endif

  coralmicro::JsonRpcHttpServer server;
  server.AddUriHandler(UriHandler);
  coralmicro::UseHttpServer(&server);
  vTaskSuspend(nullptr);
}

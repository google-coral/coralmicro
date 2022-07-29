/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIBS_TESTLIB_TEST_LIB_H_
#define LIBS_TESTLIB_TEST_LIB_H_

#include <string>
#include <vector>

#include "third_party/mjson/src/mjson.h"

namespace coralmicro::testlib {

inline constexpr char kMethodGetSerialNumber[] = "get_serial_number";
inline constexpr char kMethodRunTestConv1[] = "run_testconv1";
inline constexpr char kMethodSetTPUPowerState[] = "set_tpu_power_state";
inline constexpr char kMethodPosenetStressRun[] = "posenet_stress_run";
inline constexpr char kMethodBeginUploadResource[] = "begin_upload_resource";
inline constexpr char kMethodUploadResourceChunk[] = "upload_resource_chunk";
inline constexpr char kMethodDeleteResource[] = "delete_resource";
inline constexpr char kMethodFetchResource[] = "fetch_resource";
inline constexpr char kMethodRunClassificationModel[] =
    "run_classification_model";
inline constexpr char kMethodRunDetectionModel[] = "run_detection_model";
inline constexpr char kMethodRunSegmentationModel[] = "run_segmentation_model";
inline constexpr char kMethodStartM4[] = "start_m4";
inline constexpr char kMethodCaptureTestPattern[] = "capture_test_pattern";
inline constexpr char kMethodGetTemperature[] = "get_temperature";
inline constexpr char kMethodCaptureAudio[] = "capture_audio";
inline constexpr char kMethodWiFiSetAntenna[] = "wifi_set_antenna";
inline constexpr char kMethodWiFiScan[] = "wifi_scan";
inline constexpr char kMethodWiFiConnect[] = "wifi_connect";
inline constexpr char kMethodWiFiDisconnect[] = "wifi_disconnect";
inline constexpr char kMethodWiFiGetIp[] = "wifi_get_ip";
inline constexpr char kMethodWiFiGetStatus[] = "wifi_get_status";
inline constexpr char kMethodCryptoInit[] = "a71ch_init";
inline constexpr char kMethodCryptoGetUId[] = "a71ch_get_uid";
inline constexpr char kMethodCryptoGetRandomBytes[] = "a71ch_get_random";
inline constexpr char kMethodCryptoGetSha256[] = "a71ch_get_sha_256";
inline constexpr char kMethodCryptoGetPubEccKey[] = "a71ch_get_public_ecc_key";
inline constexpr char kMethodCryptoGetEccSignature[] =
    "a71ch_get_ecc_signature";
inline constexpr char kMethodCryptoEccVerify[] = "a71ch_ecc_verify";
inline constexpr char kMethodBleScan[] = "ble_scan";

void GetSerialNumber(struct jsonrpc_request* request);
void RunTestConv1(struct jsonrpc_request* request);
void SetTPUPowerState(struct jsonrpc_request* request);
void BeginUploadResource(struct jsonrpc_request* request);
void UploadResourceChunk(struct jsonrpc_request* request);
void DeleteResource(struct jsonrpc_request* request);
void FetchResource(struct jsonrpc_request* request);
void PosenetStressRun(struct jsonrpc_request* request);
void RunClassificationModel(struct jsonrpc_request* request);
void RunSegmentationModel(struct jsonrpc_request* request);
void RunDetectionModel(struct jsonrpc_request* request);
void StartM4(struct jsonrpc_request* request);
void GetTemperature(struct jsonrpc_request* request);
void CaptureTestPattern(struct jsonrpc_request* request);
void CaptureAudio(struct jsonrpc_request* request);
void WiFiSetAntenna(struct jsonrpc_request* request);
void WiFiScan(struct jsonrpc_request* request);
void WiFiConnect(struct jsonrpc_request* request);
void WiFiDisconnect(struct jsonrpc_request* request);
void WiFiGetIp(struct jsonrpc_request* request);
void WiFiGetStatus(struct jsonrpc_request* request);
void CryptoInit(struct jsonrpc_request* request);
void CryptoGetUID(struct jsonrpc_request* request);
void CryptoGetRandomBytes(struct jsonrpc_request* request);
void CryptoGetSha256(struct jsonrpc_request* request);
void CryptoGetPublicEccKey(struct jsonrpc_request* request);
void CryptoGetEccSignature(struct jsonrpc_request* request);
void CryptoEccVerify(struct jsonrpc_request* request);
void BleScan(struct jsonrpc_request* request);
}  // namespace coralmicro::testlib

#endif  // LIBS_TESTLIB_TEST_LIB_H_

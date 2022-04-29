#ifndef _LIBS_TESTLIB_TEST_LIB_H_
#define _LIBS_TESTLIB_TEST_LIB_H_

#include "third_party/mjson/src/mjson.h"

#include <string>
#include <vector>

namespace coral::micro {
namespace testlib {

inline constexpr char kMethodGetSerialNumber[] = "get_serial_number";
inline constexpr char kMethodRunTestConv1[] = "run_testconv1";
inline constexpr char kMethodSetTPUPowerState[] = "set_tpu_power_state";
inline constexpr char kMethodPosenetStressRun[] = "posenet_stress_run";
inline constexpr char kMethodBeginUploadResource[] = "begin_upload_resource";
inline constexpr char kMethodUploadResourceChunk[] = "upload_resource_chunk";
inline constexpr char kMethodDeleteResource[] = "delete_resource";
inline constexpr char kMethodRunClassificationModel[] = "run_classification_model";
inline constexpr char kMethodRunDetectionModel[] = "run_detection_model";
inline constexpr char kMethodStartM4[] = "start_m4";
inline constexpr char kMethodCaptureTestPattern[] = "capture_test_pattern";
inline constexpr char kMethodGetTemperature[] = "get_temperature";
inline constexpr char kMethodCaptureAudio[] = "capture_audio";
inline constexpr char kMethodWifiSetAntenna[] = "wifi_set_antenna";

bool JsonRpcGetIntegerParam(struct jsonrpc_request* request, const char* param_name, int* out);
bool JsonRpcGetBooleanParam(struct jsonrpc_request* request, const char* param_name, bool* out);
bool JsonRpcGetStringParam(struct jsonrpc_request* request, const char* param_name, std::string* out);
bool JsonRpcGetBase64Param(struct jsonrpc_request* request, const char *param_name, std::vector<uint8_t>* out);

void GetSerialNumber(struct jsonrpc_request* request);
void RunTestConv1(struct jsonrpc_request* request);
void SetTPUPowerState(struct jsonrpc_request* request);
void BeginUploadResource(struct jsonrpc_request* request);
void UploadResourceChunk(struct jsonrpc_request* request);
void DeleteResource(struct jsonrpc_request* request);
void RunClassificationModel(struct jsonrpc_request* request);
void RunDetectionModel(struct jsonrpc_request* request);
void StartM4(struct jsonrpc_request* request);
void GetTemperature(struct jsonrpc_request* request);
void CaptureTestPattern(struct jsonrpc_request* request);
void CaptureAudio(struct jsonrpc_request* request);
void WifiSetAntenna(struct jsonrpc_request *request);

}  // namespace testlib
}  // namespace coral::micro

#endif  // _LIBS_TESTLIB_TEST_LIB_H_

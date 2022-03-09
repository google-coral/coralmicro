#ifndef _LIBS_TESTLIB_TEST_LIB_H_
#define _LIBS_TESTLIB_TEST_LIB_H_

#include "third_party/mjson/src/mjson.h"

#include <vector>

namespace valiant {
namespace testlib {

// TODO(dkovalev): Add inline after switching to C++17.
constexpr char kMethodGetSerialNumber[] = "get_serial_number";
constexpr char kMethodRunTestConv1[] = "run_testconv1";
constexpr char kMethodSetTPUPowerState[] = "set_tpu_power_state";
constexpr char kMethodPosenetStressRun[] = "posenet_stress_run";
constexpr char kMethodBeginUploadResource[] = "begin_upload_resource";
constexpr char kMethodUploadResourceChunk[] = "upload_resource_chunk";
constexpr char kMethodDeleteResource[] = "delete_resource";
constexpr char kMethodRunClassificationModel[] = "run_classification_model";
constexpr char kMethodRunDetectionModel[] = "run_detection_model";
constexpr char kMethodStartM4[] = "start_m4";
constexpr char kMethodCaptureTestPattern[] = "capture_test_pattern";
constexpr char kMethodGetTemperature[] = "get_temperature";
constexpr char kMethodCaptureAudio[] = "capture_audio";
constexpr char kMethodWifiSetAntenna[] = "wifi_set_antenna";

bool JSONRPCGetIntegerParam(struct jsonrpc_request* request, const char* param_name, int* out);
bool JSONRPCGetBooleanParam(struct jsonrpc_request* request, const char* param_name, bool* out);
bool JSONRPCGetStringParam(struct jsonrpc_request* request, const char* param_name, std::vector<char>* out);

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
}  // namespace valiant

#endif  // _LIBS_TESTLIB_TEST_LIB_H_

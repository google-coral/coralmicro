#ifndef _LIBS_TESTLIB_TEST_LIB_H_
#define _LIBS_TESTLIB_TEST_LIB_H_

#include "third_party/mjson/src/mjson.h"

#include <vector>

namespace valiant {
namespace testlib {

constexpr const char *kBeginUploadResourceName = "begin_upload_resource";
constexpr const char *kUploadResourceChunkName = "upload_resource_chunk";
constexpr const char *kDeleteResourceName = "delete_resource";
constexpr const char *kRunClassificationModelName = "run_classification_model";

bool JSONRPCGetIntegerParam(struct jsonrpc_request* request, const char *param_name, int* out);
bool JSONRPCGetBooleanParam(struct jsonrpc_request* request, const char *param_name, bool *out);
bool JSONRPCGetStringParam(struct jsonrpc_request* request, const char *param_name, std::vector<char>* out);

void GetSerialNumber(struct jsonrpc_request *request);
void RunTestConv1(struct jsonrpc_request *request);
void SetTPUPowerState(struct jsonrpc_request *request);
void BeginUploadResource(struct jsonrpc_request *request);
void UploadResourceChunk(struct jsonrpc_request *request);
void DeleteResource(struct jsonrpc_request *request);
void RunClassificationModel(struct jsonrpc_request *request);

}  // namespace testlib
}  // namespace valiant

#endif  // _LIBS_TESTLIB_TEST_LIB_H_

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

#include "libs/rpc/rpc_utils.h"

#include <memory>

#include "third_party/mjson/src/mjson.h"

namespace coralmicro {

namespace {

std::unique_ptr<char[]> JsonRpcCreateParamFormatString(const char* param_name) {
  const char* param_format = "$[0].%s";
  // +1 for null terminator.
  auto size = snprintf(nullptr, 0, param_format, param_name) + 1;
  auto param_pattern = std::make_unique<char[]>(size);
  snprintf(param_pattern.get(), size, param_format, param_name);
  return param_pattern;
}

}  // namespace

void JsonRpcReturnBadParam(struct jsonrpc_request* request, const char* message,
                           const char* param_name) {
  jsonrpc_return_error(request, JSONRPC_ERROR_BAD_PARAMS, message, "{%Q:%Q}",
                       "param", param_name);
}

bool JsonRpcGetIntegerParam(struct jsonrpc_request* request,
                            const char* param_name, int* out) {
  auto param_pattern = JsonRpcCreateParamFormatString(param_name);

  double value;
  if (mjson_get_number(request->params, request->params_len,
                       param_pattern.get(), &value) == 0) {
    JsonRpcReturnBadParam(request, "invalid param", param_name);
    return false;
  }

  *out = static_cast<int>(value);
  return true;
}

bool JsonRpcGetBooleanParam(struct jsonrpc_request* request,
                            const char* param_name, bool* out) {
  auto param_pattern = JsonRpcCreateParamFormatString(param_name);

  int value;
  if (mjson_get_bool(request->params, request->params_len, param_pattern.get(),
                     &value) == 0) {
    JsonRpcReturnBadParam(request, "invalid param", param_name);
    return false;
  }

  *out = static_cast<bool>(value);
  return true;
}

bool JsonRpcGetStringParam(struct jsonrpc_request* request,
                           const char* param_name, std::string* out) {
  auto param_pattern = JsonRpcCreateParamFormatString(param_name);

  ssize_t size = 0;
  int tok = mjson_find(request->params, request->params_len,
                       param_pattern.get(), nullptr, &size);
  if (tok != MJSON_TOK_STRING) {
    JsonRpcReturnBadParam(request, "invalid param", param_name);
    return false;
  }

  out->resize(size);
  auto len = mjson_get_string(request->params, request->params_len,
                              param_pattern.get(), out->data(), out->size());
  out->resize(len);
  return true;
}

bool JsonRpcGetBase64Param(struct jsonrpc_request* request,
                           const char* param_name, std::vector<uint8_t>* out) {
  auto param_pattern = JsonRpcCreateParamFormatString(param_name);

  ssize_t size = 0;
  int tok = mjson_find(request->params, request->params_len,
                       param_pattern.get(), nullptr, &size);
  if (tok != MJSON_TOK_STRING) {
    JsonRpcReturnBadParam(request, "invalid param", param_name);
    return false;
  }

  // `size` includes both quotes, `size - 2` is the real string size. Base64
  // encodes every 3 bytes as 4 chars. Buffer size of `3 * ceil(size - 2) / 4`
  // should be enough.
  out->resize(3 * (((size - 2) + 3) / 4));
  auto len = mjson_get_base64(request->params, request->params_len,
                              param_pattern.get(),
                              reinterpret_cast<char*>(out->data()), size);
  out->resize(len);
  return true;
}

}  // namespace coralmicro
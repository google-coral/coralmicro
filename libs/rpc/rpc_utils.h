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

#ifndef LIBS_RPC_RPC_UTILS_H_
#define LIBS_RPC_RPC_UTILS_H_

#include <cstdint>
#include <string>
#include <vector>

#include "third_party/mjson/src/mjson.h"

namespace coralmicro {

// Response a JSONRPC_ERROR_BAD_PARAMS code to the requester.
//
// @param request The request to response to.
// @param message The message to send back to the requester.
// @param param_name The name of the bad param.
void JsonRpcReturnBadParam(struct jsonrpc_request* request, const char* message,
                           const char* param_name);

// Gets an integer param from RPC request.
//
// @param request The request to parse the integer.
// @param param_name The name of the parameter to parse.
// @param out The integer to return the value to.
// @returns True if the param were parsed successfully, else False.
bool JsonRpcGetIntegerParam(struct jsonrpc_request* request,
                            const char* param_name, int* out);
// Gets a boolean param from RPC request.
//
// @param request The request to parse the boolean.
// @param param_name The name of the parameter to parse.
// @param out The boolean to return the value to.
// @returns True if the param were parsed successfully, else False.
bool JsonRpcGetBooleanParam(struct jsonrpc_request* request,
                            const char* param_name, bool* out);

// Gets a string param from RPC request.
//
// @param request The request to parse the string.
// @param param_name The name of the parameter to parse.
// @param out The string to return the value to.
// @returns True if the param were parsed successfully, else False.
bool JsonRpcGetStringParam(struct jsonrpc_request* request,
                           const char* param_name, std::string* out);

// Gets a base64 encoded string param from RPC request.
//
// @param request The request to parse the string.
// @param param_name The name of the parameter to parse.
// @param out The output array to return the value to.
// @returns True if the param were parsed successfully, else False.
bool JsonRpcGetBase64Param(struct jsonrpc_request* request,
                           const char* param_name, std::vector<uint8_t>* out);

}  // namespace coralmicro

#endif  // define LIBS_RPC_RPC_UTILS_H_

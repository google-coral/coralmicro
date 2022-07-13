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

#include "libs/base/strings.h"

namespace coralmicro {

std::string StrToHex(const char* src, size_t src_len) {
    std::string output;
    output.reserve(src_len * 2);
    for (size_t dest_idx = 0, src_idx = 0; src_idx < src_len;
         dest_idx += 2, src_idx += 1) {
        sprintf(&output[dest_idx], "%02x", src[src_idx]);
    }
    return output;
}

std::string StrToHex(const std::string& src) {
    return StrToHex(src.data(), src.size());
}
}  // namespace coralmicro

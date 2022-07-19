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
std::string StrToHex(const char* s, size_t size) {
  static constexpr char kHexChars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                       '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  std::string hex;
  hex.resize(2 * size);
  for (size_t i = 0, j = 0; i < size; ++i) {
    hex[j++] = kHexChars[s[i] >> 4];
    hex[j++] = kHexChars[s[i] & 0xF];
  }
  return hex;
}
}  // namespace coralmicro

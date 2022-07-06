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

#ifndef LIBS_A71CH_A71CH_H_
#define LIBS_A71CH_A71CH_H_

#include <optional>
#include <string>

namespace coralmicro::a71ch {

// Initializes the a71ch module.
//
// This function must be called before using any of our a71ch wrapper functions
// or the direct a71ch api. The reference manual for the a71ch chip can be found
// here: https://www.nxp.com/a71ch
bool Init();

// Convenience helper to get the uid of the a71ch module.
//
// @return the unique id of this a71ch module or std::nullopt.
std::optional<std::string> GetUID();
}  // namespace coralmicro::a71ch

#endif  // LIBS_A71CH_A71CH_H_

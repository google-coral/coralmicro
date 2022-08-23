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

#ifndef LIBS_BASE_RANDOM_H_
#define LIBS_BASE_RANDOM_H_

#include <cstddef>

namespace coralmicro {

// Initializes hardware random number generator.
//
// Programs on the M7 do not need to call this because it is automatically
// called internally. M7 programs can immediately call
// `RandomGenerate()`.
//
// Programs on the M4 must call this to intialize the generator before
// calling `RandomGenerate()`.
void RandomInit();

// Generates random byte sequence.
//
// @param buf Buffer to write random bytes to.
// @param size Size of the buffer.
// @returns True upon success, false otherwise.
bool RandomGenerate(void* buf, size_t size);

}  // namespace coralmicro

#endif  // LIBS_BASE_RANDOM_H_

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

#include <cstdlib>

#include "Arduino.h"

void randomSeed(unsigned long seed) {
  if (seed != 0) {
    srand48(seed);
  }
}

long random(long max) {
  if (max == 0) {
    return 0;
  }
  return lrand48() % max;
}

long random(long min, long max) {
  long diff = max - min;

  if (diff <= 0) {
    diff = 0;
  }

  return random(diff) + min;
}

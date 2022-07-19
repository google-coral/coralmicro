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

#ifndef LIBS_BASE_TEMPSENSE_H_
#define LIBS_BASE_TEMPSENSE_H_

namespace coralmicro {

// Enumerates the various temperature sensors.
enum class TempSensor {
  // The CPU temperature sensor.
  kCpu,
  // The Edge TPU temperature sensor.
  kTpu,
};

// Initializes the temperature sensors.
//
// You must call this before `TempSensorRead()` to ensure that the temperature
// sensors are in a known good state.
void TempSensorInit();

// Reads the temperature of a sensor.
//
// You must first call `TempSensorInit()` regardless of which sensor you're
// using.
// @param sensor The sensor to get the temperature.
// @return The actual temperature in Celcius or 0.0 on failure.
float TempSensorRead(TempSensor sensor);

}  // namespace coralmicro

#endif  // LIBS_BASE_TEMPSENSE_H_

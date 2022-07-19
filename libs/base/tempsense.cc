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

#include "libs/base/tempsense.h"

#include "libs/base/check.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_tempsensor.h"

namespace coralmicro {

void TempSensorInit() {
  // Configure CPU for just single measurement mode.
  tmpsns_config_t config;
  TMPSNS_GetDefaultConfig(&config);
  config.measureMode = kTEMPSENSOR_SingleMode;

  TMPSNS_Init(TMPSNS, &config);
}

float TempSensorRead(TempSensor sensor) {
  switch (sensor) {
    case TempSensor::kCpu: {
      // Toggle CTRL1_START to get new single-shot read.
      TMPSNS_StopMeasure(TMPSNS);
      TMPSNS_StartMeasure(TMPSNS);
      return TMPSNS_GetCurrentTemperature(TMPSNS);
    }
    case TempSensor::kTpu: {
      auto context = EdgeTpuManager::GetSingleton()->OpenDevice();
      return EdgeTpuManager::GetSingleton()->GetTemperature().value();
    }
    default:
      CHECK(!"Invalid sensor value");
      return 0.0f;
  }
}

}  // namespace coralmicro

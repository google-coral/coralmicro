#include "libs/base/tempsense.h"

#include "libs/base/check.h"
#include "libs/tpu/edgetpu_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_tempsensor.h"

namespace coral::micro::tempsense {

void Init() {
    // Configure CPU for just single measurement mode.
    tmpsns_config_t config;
    TMPSNS_GetDefaultConfig(&config);
    config.measureMode = kTEMPSENSOR_SingleMode;

    TMPSNS_Init(TMPSNS, &config);
}

float GetTemperature(TempSensor sensor) {
    switch (sensor) {
        case TempSensor::kCPU: {
            // Toggle CTRL1_START to get new single-shot read.
            TMPSNS_StopMeasure(TMPSNS);
            TMPSNS_StartMeasure(TMPSNS);
            return TMPSNS_GetCurrentTemperature(TMPSNS);
        }
        case TempSensor::kTPU: {
            auto context = EdgeTpuManager::GetSingleton()->OpenDevice();
            return EdgeTpuManager::GetSingleton()->GetTemperature().value();
        }
        default:
            CHECK(!"Invalid sensor value");
            return 0.0f;
    }
}

}  // namespace coral::micro::tempsense
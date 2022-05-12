#include "libs/base/tempsense.h"

#include "libs/tpu/edgetpu_manager.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_tempsensor.h"

namespace coral::micro {
namespace tempsense {

void Init() {
    /* Configure CPU for just single measurement mode. */
    tmpsns_config_t config;
    TMPSNS_GetDefaultConfig(&config);
    config.measureMode = kTEMPSENSOR_SingleMode;

    TMPSNS_Init(TMPSNS, &config);
}

float GetTemperature(TempSensor sensor) {
    float temperature;
    switch (sensor) {
        case TempSensor::kCPU:
            /* Toggle CTRL1_START to get new single-shot read */
            TMPSNS_StopMeasure(TMPSNS);
            TMPSNS_StartMeasure(TMPSNS);
            temperature = TMPSNS_GetCurrentTemperature(TMPSNS);
            return temperature;
        case TempSensor::kTPU: {
            const auto& context = EdgeTpuManager::GetSingleton()->OpenDevice();
            return EdgeTpuManager::GetSingleton()->GetTemperature();
        }
        default:
            return -273.15f;
    }
}

}  // namespace tempsense
}  // namespace coral::micro
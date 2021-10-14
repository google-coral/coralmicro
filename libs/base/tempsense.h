#ifndef _LIBS_BASE_TEMPSENSE_H_
#define _LIBS_BASE_TEMPSENSE_H_

namespace valiant {
namespace tempsense {

enum class TempSensor : unsigned int {
    kCPU,
    kTPU,
    kSensorCount,
};

void Init();

/* For a given sensor, returns the temperature in celsius as a float */
float GetTemperature(TempSensor sensor);

}  // namespace tempsense
}  // namespace valiant

#endif  // _LIBS_BASE_TEMPSENSE_H_
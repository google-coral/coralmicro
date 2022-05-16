#ifndef LIBS_BASE_TEMPSENSE_H_
#define LIBS_BASE_TEMPSENSE_H_

namespace coral::micro {
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
}  // namespace coral::micro

#endif  // LIBS_BASE_TEMPSENSE_H_

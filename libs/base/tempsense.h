#ifndef LIBS_BASE_TEMPSENSE_H_
#define LIBS_BASE_TEMPSENSE_H_

namespace coral::micro::tempsense {

// Enumerates the various temperature sensors.
enum class TempSensor : unsigned int {
    // The CPU temperature sensor.
    kCPU,
    // The Edge TPU temperature sensor.
    kTPU,
    // The number of temperature sensors.
    kSensorCount,
};

// Initializes the temperature sensors.
//
// You must call this before `GetTemperature()` to ensure that the temperature sensors
// are in a known good state.
void Init();

// Gets the temperature of a sensor.
//
// You must first call `Init()` regardless of which sensor you're using.
// @param sensor The sensor to get the temperature.
// @return The actual temperature in Celcius or 0.0 on failure.
float GetTemperature(TempSensor sensor);

}  // namespace coral::micro::tempsense

#endif  // LIBS_BASE_TEMPSENSE_H_

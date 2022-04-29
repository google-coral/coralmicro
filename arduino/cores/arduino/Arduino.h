#ifndef Arduino_h
#define Arduino_h

#include <api/ArduinoAPI.h>
#include "third_party/freertos_kernel/include/FreeRTOS.h"
#include "third_party/nxp/rt1176-sdk/devices/MIMXRT1176/drivers/fsl_pxp.h"
#include "HardwareSerial.h"
#include "pins_arduino.h"

#define interrupts() portENABLE_INTERRUPTS()
#define noInterrupts() portDISABLE_INTERRUPTS()

void analogReadResolution(int bits);
void analogWriteResolution(int bits);

extern coral::micro::arduino::HardwareSerial Serial;

#endif  // Arduino_h
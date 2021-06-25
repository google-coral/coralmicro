#ifndef __PINS_ARDUINO_H__
#define __PINS_ARDUINO_H__

/*
Valiant Arduino pin assignment diagram:
Pin (Analog) | Silk label | Silk label | Pin (Analog)

-              VSYS         SCL1         16
0              SCL6         SDA1         15
1 (A3)         PWM0         SDA6         14
2 (A4)         PWM1         DAC          13 (A2)
3              RTS          SDI          12
4              CTS          SDO          11
5              RXD          SCK          10
6              TXD          CS           9
7 (A0)         A-B          VBAK         -
8 (A1)         A-A          3V3          -
-              1V8          1V8          -
-              GND          GND          -

17 - LED
18 - User button
*/

#define PIN_BTN (18U)

#define PIN_LED (17U)
#define LED_BUILTIN (PIN_LED)
#define LEDG (PIN_LED)

#define D0 (0U)

#define A0 (7U)
#define A1 (8U)
#define A2 (13U)
#define A3 (1U)
#define A4 (2U)

#define DAC0 (13U)

#endif  // __PINS_ARDUINO_H__
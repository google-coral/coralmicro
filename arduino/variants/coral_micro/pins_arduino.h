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

#ifndef __PINS_ARDUINO_H__
#define __PINS_ARDUINO_H__

/*
Coral Dev Board Micro Arduino pin assignment diagram:
Pin (Arduino pin) | Silk label | Silk label | Pin (Arduino pin)

-              VSYS         SCL1         16
0 (D0)         SCL6         SDA1         15
1 (A3)         PWM0         SDA6         14 (D3)
2 (A4)         PWM1         DAC          13 (A2)
3 (D1)         RTS          SDI          12
4 (D2)         CTS          SDO          11
5              RXD          SCK          10
6              TXD          CS           9
7 (A0)         A-B          VBAK         -
8 (A1)         A-A          3V3          -
-              1V8          1V8          -
-              GND          GND          -

17 - LED
18 - User button (D4)
19 - Power LED
20 - Tpu LED
*/

#define PIN_BTN (18U)

#define PIN_LED_USER (17U)
#define LED_BUILTIN (PIN_LED_USER)
#define LEDG (PIN_LED_USER)

#define PIN_LED_STATUS (19U)
#define PIN_LED_TPU (20U)

#define D0 (0U)
#define D1 (3U)
#define D2 (4U)
#define D3 (14U)
#define D4 (18U)

#define A0 (7U)
#define A1 (8U)
#define A2 (13U)
#define A3 (1U)
#define A4 (2U)

#define DAC0 (PIN_BTN)

#define digitalPinToInterrupt(p) (p)

#endif  // __PINS_ARDUINO_H__

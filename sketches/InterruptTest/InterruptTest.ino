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

/*
  InterruptTest

  Prints a counter for each pin interrupt

  This example code is in the public domain.

*/
// Board pin
const int button_pin_0 = D0;  // SCL6
const int button_pin_1 = D1;  // RTS
const int button_pin_2 = D2;  // CTS
const int button_pin_3 = D3;  // SDA6
const int button_pin_4 = D4;  // User button
volatile int counter_0 = 0;
volatile int counter_1 = 0;
volatile int counter_2 = 0;
volatile int counter_3 = 0;
volatile int counter_4 = 0;

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino Interrupt Test!");

  pinMode(button_pin_0, INPUT);
  pinMode(button_pin_1, INPUT_PULLDOWN);
  pinMode(button_pin_2, INPUT_PULLUP);
  pinMode(button_pin_3, INPUT);
  pinMode(button_pin_4, INPUT);
  attachInterrupt(digitalPinToInterrupt(button_pin_0), pin0_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_pin_1), pin1_ISR, HIGH);
  attachInterrupt(digitalPinToInterrupt(button_pin_2), pin2_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(button_pin_3), pin3_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(button_pin_4), pin4_ISR, RISING);
}

// the loop function runs over and over again forever
void loop() {
  Serial.print("isr0 counter: ");
  Serial.println(counter_0);
  Serial.print("isr1 counter: ");
  Serial.println(counter_1);
  Serial.print("isr2 counter: ");
  Serial.println(counter_2);
  Serial.print("isr3 counter: ");
  Serial.println(counter_3);
  Serial.print("isr counter_4: ");
  Serial.println(counter_4);
  Serial.println("=================");
  delay(500);
}

void pin0_ISR() { counter_0++; }

void pin1_ISR() { counter_1++; }

void pin2_ISR() { counter_2++; }

void pin3_ISR() { counter_3++; }

void pin4_ISR() { counter_4++; }

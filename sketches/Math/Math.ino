#include "Arduino.h"

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to shows board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Coral Micro Arduino !");
}

void loop() {
  Serial.print("abs(4): ");
  Serial.println(abs(4));
  Serial.print("abs(-4): ");
  Serial.println(abs(-4));

  Serial.print("contrain 4 to [1,3]: ");
  Serial.println(constrain(4, 1, 3));
  Serial.print("contrain -4 to [1,3]: ");
  Serial.println(constrain(-4, 1, 3));

  Serial.print("map(512, 0, 1023, 0, 254): ");
  Serial.println(map(512, 0, 1023, 0, 254));

  Serial.print("max [-2,2]: ");
  Serial.println(max(-2, 2));
  Serial.print("min [-2,2]: ");
  Serial.println(min(-2, 2));

  Serial.print("pow (2,2): ");
  Serial.println(pow(2, 2));

  Serial.print("sq(2): ");
  Serial.println(sq(2));

  Serial.print("sqrt 4: ");
  Serial.println(sqrt(4));

  Serial.print("sin(0): ");
  Serial.println(sin(0));
  Serial.print("sin(pi/4): ");
  Serial.println(sin(PI / 4));
  Serial.print("sin(pi/2): ");
  Serial.println(sin(PI / 2));

  Serial.print("cos(0): ");
  Serial.println(cos(0));
  Serial.print("cos(pi/4): ");
  Serial.println(cos(PI / 4));
  Serial.print("cos(pi/2): ");
  Serial.println(cos(PI / 2));

  Serial.print("tan(0): ");
  Serial.println(tan(0));
  Serial.print("tan(pi/4): ");
  Serial.println(tan(PI / 4));

  delay(10000);
}
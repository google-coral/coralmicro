/*
  InterruptTest

  Prints a counter for each pin interrupt

  This example code is in the public domain.

*/
                           // Board pin
const int buttonPin0 = D0; // SCL6
const int buttonPin1 = D1; // RTS
const int buttonPin2 = D2; // CTS
const int buttonPin3 = D3; // SDA6
const int buttonPin4 = D4; // User button
volatile int counter0 = 0;
volatile int counter1 = 0;
volatile int counter2 = 0;
volatile int counter3 = 0;
volatile int counter4 = 0;

void setup() {
  pinMode(buttonPin0, INPUT);
  pinMode(buttonPin1, INPUT_PULLDOWN);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT);
  pinMode(buttonPin4, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin0), pin0_ISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(buttonPin1), pin1_ISR, HIGH);
  attachInterrupt(digitalPinToInterrupt(buttonPin2), pin2_ISR, LOW);
  attachInterrupt(digitalPinToInterrupt(buttonPin3), pin3_ISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(buttonPin4), pin4_ISR, RISING);
}

// the loop function runs over and over again forever
void loop() {
  Serial.print("isr0 counter: ");
  Serial.println(counter0);
  Serial.print("isr1 counter: ");
  Serial.println(counter1);
  Serial.print("isr2 counter: ");
  Serial.println(counter2);
  Serial.print("isr3 counter: ");
  Serial.println(counter3);
  Serial.print("isr counter4: ");
  Serial.println(counter4);
  Serial.println("=================");
  delay(500);
}

void pin0_ISR() {
  counter0++;
}

void pin1_ISR() {
  counter1++;
}

void pin2_ISR() {
  counter2++;
}

void pin3_ISR() {
  counter3++;
}

void pin4_ISR() {
  counter4++;
}


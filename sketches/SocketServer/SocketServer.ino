#include "Arduino.h"
#include "SocketClient.h"
#include "SocketServer.h"

// Simple example showing the usage of `SocketServer`.
// After flashing this to the board, you can connect to the
// server on port 31337 via the USB connection, and your input
// will be echoed on the server console.
//
// Ex: `nc 10.10.10.1 31337`

namespace {

coralmicro::arduino::SocketClient client;
coralmicro::arduino::SocketServer server(31337);

}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino SocketServer!");
  pinMode(PIN_LED_USER, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  // Turn on Status LED to show the board is on.
  digitalWrite(PIN_LED_STATUS, HIGH);

  server.begin();
  Serial.println("Server ready on port 31337");
  client = server.available();
}

void loop() {
  if (client.available()) {
    Serial.write(client.read());
    Serial.flush();
  }
}

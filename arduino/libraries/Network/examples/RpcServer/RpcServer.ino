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

// Takes an image and save it in the filesystem when you press the user button
// on the Coral Dev Board Micro.

#include <Arduino.h>

#include <cstdint>
#include <memory>

#include "libs/base/utils.h"
#include "libs/rpc/rpc_http_server.h"

// This is the equivalent arduino sketch for examples/rpc_server. Upload
// this sketch and then trigger an RPC request over USB from a Linux computer:
//
//    python3 -m pip install -r examples/rpc_server/requirements.txt
//    python3 examples/rpc_server/rpc_client.py

void SerialNumber(struct jsonrpc_request* r) {
  auto serial = coralmicro::GetSerialNumber();
  jsonrpc_return_success(r, "{%Q:%.*Q}", "serial_number", serial.size(),
                         serial.c_str());
}

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino RPC Server!");

  // Set up an RPC server that serves the latest image.
  jsonrpc_export("serial_number", SerialNumber);
  coralmicro::UseHttpServer(new coralmicro::JsonRpcHttpServer);
}

void loop() {}

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

// Reads and writes files to the filesystem on flash storage.

#include <coralmicro_SD.h>

void setup() {
  Serial.begin(115200);
  // Turn on Status LED to show the board is on.
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, HIGH);
  Serial.println("Arduino SD Filesystem!");

  SD.begin();

  Serial.println("Begin SD example");
  // Print out the directory tree
  printDirectoryTree();
  Serial.println("Should be empty");

  // Create individual directory
  assert(SD.mkdir("/dir"));

  // Print out the directory tree
  printDirectoryTree();

  // Open a file for writing
  SDFile file = SD.open("/dir/file", FILE_WRITE);

  // Write to the file, and check
  auto size = file.println("Hello World!");  // adds \r\n

  // Read the file
  char readbuf[255];
  file.seek(0);
  size = file.read(readbuf, sizeof(readbuf) - 1);
  readbuf[size] = 0;
  Serial.print(readbuf);

  // Close the file
  file.close();
  assert(!file);

  // Remove all files and directories
  assert(SD.remove("/dir/file"));
  assert(SD.rmdir("/dir"));

  // Print out the directory tree
  printDirectoryTree();
  Serial.println("Should be empty");

  Serial.println("End SD test");
}

void loop() { delay(100); }

void printDirectoryTree() {
  Serial.println("Current directory tree");
  SDFile dir = SD.open("/");
  printDirectory(dir, 0);
  dir.close();
}

void printDirectory(SDFile dir, int numTabs) {
  // Prints a directory and all its subdirs
  while (true) {
    SDFile entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size());
    }
    entry.close();
  }
}

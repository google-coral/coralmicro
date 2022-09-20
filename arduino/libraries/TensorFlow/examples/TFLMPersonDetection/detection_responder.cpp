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

#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/detection_responder.h"

#include <Arduino.h>
#include <coral_micro.h>

void RespondToDetection(tflite::ErrorReporter* error_reporter,
                        int8_t person_score, int8_t no_person_score) {
  TF_LITE_REPORT_ERROR(error_reporter, "person_score: %d no_person_score: %d",
                       person_score, no_person_score);
  digitalWrite(PIN_LED_USER, person_score > no_person_score ? HIGH : LOW);
}

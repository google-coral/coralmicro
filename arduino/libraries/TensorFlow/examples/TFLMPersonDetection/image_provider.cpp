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

#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/image_provider.h"

#include <coral_micro.h>
#include <coralmicro_camera.h>

using namespace coralmicro::arduino;

TfLiteStatus GetImage(tflite::ErrorReporter*, int, int, int,
                      int8_t* image_data) {
  FrameBuffer fb;
  if (Camera.grab(fb) != CameraStatus::SUCCESS) return kTfLiteError;
  for (int i = 0; i < fb.getBufferSize(); ++i) {
    image_data[i] = fb.getBuffer()[i] - 128;
  }
  return kTfLiteOk;
}

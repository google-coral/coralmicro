// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "third_party/tflite-micro/tensorflow/lite/micro/examples/person_detection/image_provider.h"

#include <memory>

#include "libs/camera/camera.h"

TfLiteStatus GetImage(tflite::ErrorReporter* error_reporter, int image_width,
                      int image_height, int channels, int8_t* image_data) {
  auto unsigned_image_data =
      std::make_unique<uint8_t[]>(image_width * image_height);
  coralmicro::CameraFrameFormat fmt;
  fmt.width = image_width;
  fmt.height = image_height;
  fmt.fmt = coralmicro::CameraFormat::kY8;
  fmt.filter = coralmicro::CameraFilterMethod::kBilinear;
  fmt.preserve_ratio = false;
  fmt.buffer = unsigned_image_data.get();
  bool ret = coralmicro::CameraTask::GetSingleton()->GetFrame({fmt});
  for (int i = 0; i < image_width * image_height; ++i) {
    image_data[i] = unsigned_image_data[i] - 128;
  }
  return ret ? kTfLiteOk : kTfLiteError;
}

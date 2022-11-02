# Copyright 2022 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_executable_m7(tflm_person_detection_m7
    image_provider.cc
    main_functions.cc
    person_detection.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/person_detect_model.tflite
)

target_link_libraries(tflm_person_detection_m7
    libs_base-m7_freertos
)

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

add_executable_m7(classify_speech
    classify_speech.cc
    DATA
    ${CMAKE_SOURCE_DIR}/models/voice_commands_v0.7_edgetpu.tflite
    ${CMAKE_SOURCE_DIR}/models/labels_gc2.raw.txt
)

target_link_libraries(classify_speech
    libs_base-m7_freertos
    libs_audio_freertos
)

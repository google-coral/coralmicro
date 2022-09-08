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

# set(YAMNET_CPU true) # To run YamNet on the CPU, uncomment.
if(YAMNET_CPU)
    add_compile_definitions(YAMNET_CPU)
    set(model_name yamnet_spectra_in.tflite)
else()
    set(model_name yamnet_spectra_in_edgetpu.tflite)
endif()

add_executable_m7(classify_audio
    classify_audio.cc
    DATA
    ${CMAKE_SOURCE_DIR}/models/yamnet_test_audio.bin
    ${CMAKE_SOURCE_DIR}/models/${model_name}
)

target_link_libraries(classify_audio
    libs_base-m7_freertos
)

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

# set(YAMNET_CPU true) # If CPU YAMNet is desired, uncomment.
if(YAMNET_CPU)
    add_definitions(-DYAMNET_CPU)
    set(model_name yamnet.tflite)
else()
    set(model_name yamnet_edgetpu.tflite)
endif()

add_library_m7(libs_yamnet STATIC
    yamnet.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/${model_name}>/models/${model_name}
)
target_include_directories(libs_yamnet PUBLIC
    .
)
target_link_libraries(libs_yamnet PUBLIC
    libs_base-m7_freertos
    libs_flatbuffers
    libs_tensorflow-m7
)
set_target_properties(libs_yamnet PROPERTIES LINKER_LANGUAGE CXX)

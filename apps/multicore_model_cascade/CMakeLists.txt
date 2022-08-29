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

set(multicore_model_cascade_SOURCES
    multicore_model_cascade_cm7.cc
)

set(multicore_model_cascade_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_jpeg_m7
    libs_rpc_http_server
)

add_executable_m7(multicore_model_cascade
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_m4
)

target_link_libraries(multicore_model_cascade
    ${multicore_model_cascade_LINK_LIBRARIES}
)

set(multicore_model_cascade_m4_SOURCES
    multicore_model_cascade_cm4.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/person_detect_model.tflite
    ${PROJECT_SOURCE_DIR}/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite
)

set(multicore_model_cascade_m4_LINK_LIBRARIES
    libs_base-m4_freertos
    libs_tensorflow-m4
)

add_executable_m4(multicore_model_cascade_m4
    ${multicore_model_cascade_m4_SOURCES}
)

target_link_libraries(multicore_model_cascade_m4
    ${multicore_model_cascade_m4_LINK_LIBRARIES}
)

add_executable_m7(multicore_model_cascade_wifi
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_m4
)

target_compile_definitions(multicore_model_cascade_wifi PUBLIC
    MULTICORE_MODEL_CASCADE_WIFI
)

target_link_libraries(multicore_model_cascade_wifi
    ${multicore_model_cascade_LINK_LIBRARIES}
    libs_base-m7_wifi
)

add_executable_m7(multicore_model_cascade_ethernet
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_m4
)

target_compile_definitions(multicore_model_cascade_ethernet PUBLIC
    MULTICORE_MODEL_CASCADE_ETHERNET
)

target_link_libraries(multicore_model_cascade_ethernet
    ${multicore_model_cascade_LINK_LIBRARIES}
    libs_base-m7_ethernet
)

add_executable_m4(multicore_model_cascade_demo_m4
    ${multicore_model_cascade_m4_SOURCES}
)

target_link_libraries(multicore_model_cascade_demo_m4
    ${multicore_model_cascade_m4_LINK_LIBRARIES}
)

target_compile_definitions(multicore_model_cascade_demo_m4 PUBLIC
    MULTICORE_MODEL_CASCADE_DEMO
)

add_executable_m7(multicore_model_cascade_demo
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_demo_m4
)

target_compile_definitions(multicore_model_cascade_demo PUBLIC
    MULTICORE_MODEL_CASCADE_DEMO
)

target_link_libraries(multicore_model_cascade_demo
    ${multicore_model_cascade_LINK_LIBRARIES}
)

add_executable_m7(multicore_model_cascade_demo_wifi
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_demo_m4
)

target_compile_definitions(multicore_model_cascade_demo_wifi PUBLIC
    MULTICORE_MODEL_CASCADE_DEMO
    MULTICORE_MODEL_CASCADE_WIFI
)

target_link_libraries(multicore_model_cascade_demo_wifi
    ${multicore_model_cascade_LINK_LIBRARIES}
    libs_base-m7_wifi
)

add_executable_m7(multicore_model_cascade_demo_ethernet
    ${multicore_model_cascade_SOURCES}
    M4_EXECUTABLE
    multicore_model_cascade_demo_m4
)

target_compile_definitions(multicore_model_cascade_demo_ethernet PUBLIC
    MULTICORE_MODEL_CASCADE_DEMO
    MULTICORE_MODEL_CASCADE_ETHERNET
)

target_link_libraries(multicore_model_cascade_demo_ethernet
    ${multicore_model_cascade_LINK_LIBRARIES}
    libs_base-m7_ethernet
)

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

set(OOBE_SOURCES
    jpeg.cc
    oobe_cm7.cc
)

set(OOBE_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_base-m7_network
    libs_libjpeg
    libs_rpc_http_server
)

add_executable_m7(OOBE
    ${OOBE_SOURCES}
    M4_EXECUTABLE
    OOBEM4
)

target_link_libraries(OOBE
    ${OOBE_LINK_LIBRARIES}
)

set(OOBE_M4_SOURCES
    image_provider.cc
    oobe_cm4.cc
    main_functions.cc
    DATA
    ${PROJECT_SOURCE_DIR}/models/person_detect_model.tflite
    ${PROJECT_SOURCE_DIR}/models/posenet_mobilenet_v1_075_324_324_16_quant_decoder_edgetpu.tflite
)

set(OOBE_M4_LINK_LIBRARIES
    libs_base-m4_freertos
    libs_tensorflow-m4
)

add_executable_m4(OOBEM4
    ${OOBE_M4_SOURCES}
)

target_link_libraries(OOBEM4
    ${OOBE_M4_LINK_LIBRARIES}
)

add_executable_m4(OOBEDemoM4
    ${OOBE_M4_SOURCES}
)

target_link_libraries(OOBEDemoM4
    ${OOBE_M4_LINK_LIBRARIES}
)

target_compile_definitions(OOBEDemoM4 PUBLIC
    OOBE_DEMO
)

add_executable_m7(OOBEDemo
    ${OOBE_SOURCES}
    M4_EXECUTABLE
    OOBEDemoM4
)

target_compile_definitions(OOBEDemo PUBLIC
    OOBE_DEMO
)

target_link_libraries(OOBEDemo
    ${OOBE_LINK_LIBRARIES}
)

add_executable_m7(OOBEDemoWifi
    ${OOBE_SOURCES}
    M4_EXECUTABLE
    OOBEDemoM4
)

target_compile_definitions(OOBEDemoWifi PUBLIC
    OOBE_DEMO
    OOBE_DEMO_WIFI
)

target_link_libraries(OOBEDemoWifi
    ${OOBE_LINK_LIBRARIES}
    libs_base-m7_wifi
)

add_executable_m7(OOBEDemoEthernet
    ${OOBE_SOURCES}
    M4_EXECUTABLE
    OOBEDemoM4
)

target_compile_definitions(OOBEDemoEthernet PUBLIC
    OOBE_DEMO
    OOBE_DEMO_ETHERNET
)

target_link_libraries(OOBEDemoEthernet
    ${OOBE_LINK_LIBRARIES}
    libs_base-m7_ethernet
)

add_executable_m7(OOBESimple
    oobe_simple_cm7.cc
    M4_EXECUTABLE
    OOBESimpleM4
)

target_compile_definitions(OOBESimple PUBLIC
    OOBE_SIMPLE
)

target_link_libraries(OOBESimple
    libs_base-m7_freertos
)

add_executable_m4(OOBESimpleM4
    ${OOBE_M4_SOURCES}
)

target_link_libraries(OOBESimpleM4
    ${OOBE_M4_LINK_LIBRARIES}
)

target_compile_definitions(OOBESimpleM4 PUBLIC
    OOBE_SIMPLE
)

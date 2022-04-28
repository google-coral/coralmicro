set(OOBE_SOURCES
    oobe_cm7.cc
)

set(OOBE_LINK_LIBRARIES
    libs_base-m7_freertos
    libs_base-m7_http_server
    libs_posenet
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
    ${VALIANT_SOURCE_DIR}/models/person_detect_model.tflite
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

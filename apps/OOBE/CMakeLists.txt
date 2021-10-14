cmake_minimum_required(VERSION 3.13)
project(OOBE)

add_executable_m7(OOBE
    oobe_cm7.cc
    M4_EXECUTABLE
    OOBEM4
)

target_link_libraries(OOBE
    libs_base-m7_freertos
    libs_posenet
)

add_executable_m4(OOBEM4
    image_provider.cc
    oobe_cm4.cc
    main_functions.cc
    DATA
    ${VALIANT_SOURCE_DIR}/models/person_detect_model.tflite
)

target_link_libraries(OOBEM4
    libs_base-m4_freertos
    libs_tensorflow-m4
)

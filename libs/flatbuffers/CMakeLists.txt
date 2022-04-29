add_library_m7(libs_flatbuffers INTERFACE
)
target_include_directories(libs_flatbuffers INTERFACE
    ${PROJECT_SOURCE_DIR}/third_party/flatbuffers/include
)

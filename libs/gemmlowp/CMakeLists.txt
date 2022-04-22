add_library_m7(libs_gemmlowp INTERFACE
)
target_include_directories(libs_gemmlowp INTERFACE
    ${VALIANT_SOURCE_DIR}/third_party/gemmlowp
)

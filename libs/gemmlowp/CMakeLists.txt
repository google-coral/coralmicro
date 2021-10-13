cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_gemmlowp INTERFACE
)
target_include_directories(libs_gemmlowp INTERFACE
    ${VALIANT_SOURCE_DIR}/third_party/gemmlowp
)

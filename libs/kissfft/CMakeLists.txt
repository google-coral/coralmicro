cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_kissfft-m7 STATIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m7 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft
)

add_library_m4(libs_kissfft-m4 STATIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m4 PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft
)

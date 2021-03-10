cmake_minimum_required(VERSION 3.13)

add_library_m7(libs_kissfft STATIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${CMAKE_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

# target_compile_definitions(libs_kissfft PUBLIC
#     FIXED_POINT=16
# )

target_include_directories(libs_kissfft PUBLIC
    ${CMAKE_SOURCE_DIR}/third_party/kissfft
)

add_library_m7(libs_kissfft-m7 STATIC
    ${VALIANT_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${VALIANT_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m7 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/kissfft
)

add_library_m4(libs_kissfft-m4 STATIC
    ${VALIANT_SOURCE_DIR}/third_party/kissfft/kiss_fft.c
    ${VALIANT_SOURCE_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

target_include_directories(libs_kissfft-m4 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/kissfft
)

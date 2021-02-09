cmake_minimum_required(VERSION 3.13)

set(libs_CdcEem_SOURCES
    cdc_eem.cc
)

# add_library_m7(libs_CdcEem_bm STATIC
#     ${libs_CdcEem_SOURCES}
# )
# target_link_libraries(libs_CdcEem_bm
#     libs_nxp_rt1176-sdk_bm
# )

add_library_m7(libs_CdcEem_freertos STATIC
    ${libs_CdcEem_SOURCES}
)
target_link_libraries(libs_CdcEem_freertos
    libs_base-m7_freertos
)

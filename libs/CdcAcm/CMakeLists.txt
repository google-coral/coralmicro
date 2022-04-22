set(libs_CdcAcm_SOURCES
    cdc_acm.cc
)

# add_library_m7(libs_CdcAcm_bm STATIC
#     ${libs_CdcAcm_SOURCES}
# )
# target_link_libraries(libs_CdcAcm_bm
#     libs_nxp_rt1176-sdk_bm
# )

add_library_m7(libs_CdcAcm_freertos STATIC
    ${libs_CdcAcm_SOURCES}
)
target_link_libraries(libs_CdcAcm_freertos
    libs_base-m7_freertos
)

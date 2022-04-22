add_library_m7(libs_littlefs
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs PUBLIC
    LFS_THREADSAFE
)

add_library_m4(libs_littlefs-m4
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs-m4 PUBLIC
    ${VALIANT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs-m4 PUBLIC
    LFS_THREADSAFE
)

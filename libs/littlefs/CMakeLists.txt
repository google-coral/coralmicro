add_library_m7(libs_littlefs
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs PUBLIC
    LFS_THREADSAFE
)

add_library_m4(libs_littlefs-m4
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs.c
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs/lfs_util.c
)

target_include_directories(libs_littlefs-m4 PUBLIC
    ${PROJECT_SOURCE_DIR}/third_party/nxp/rt1176-sdk/middleware/littlefs
)

target_compile_definitions(libs_littlefs-m4 PUBLIC
    LFS_THREADSAFE
)

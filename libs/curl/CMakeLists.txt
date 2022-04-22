add_library_m7(libs_curl STATIC
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/altsvc.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/base64.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/conncache.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/connect.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/content_encoding.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/curl_addrinfo.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/curl_ctype.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/curl_get_line.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/dotdot.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/dynbuf.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/easy.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/escape.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/getenv.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/getinfo.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/hash.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/hostip.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/hostip4.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/hsts.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/http.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/http_chunks.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/http_proxy.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/if2ip.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/llist.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/mprintf.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/multi.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/nonblock.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/parsedate.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/progress.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/rand.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/rename.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/select.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/sendf.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/setopt.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/share.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/slist.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/speedcheck.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/splay.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/strcase.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/strdup.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/strerror.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/strtoofft.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/timeval.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/transfer.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/url.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/urlapi.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/vtls/mbedtls.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/vtls/vtls.c
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib/warnless.c
    DATA
    ${VALIANT_SOURCE_DIR}/ca-certificates.crt
)

target_compile_definitions(libs_curl PRIVATE
    BUILDING_LIBCURL
)

target_compile_definitions(libs_curl PUBLIC
    HAVE_CONFIG_H
)

target_include_directories(libs_curl PUBLIC
    ${VALIANT_SOURCE_DIR}/libs/curl
    ${VALIANT_SOURCE_DIR}/third_party/curl/include
    ${VALIANT_SOURCE_DIR}/third_party/curl/lib
)

target_link_libraries(libs_curl
    libs_base-m7_freertos
    libs_nxp_rt1176-sdk_lwip
    libs_nxp_rt1176-sdk-mbedtls
)

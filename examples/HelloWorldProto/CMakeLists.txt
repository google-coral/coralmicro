list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/third_party/nanopb/extra)
find_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS RELPATH proto
    proto/simple.proto proto/sub/unlucky.proto)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS}
    PROPERTIES GENERATED TRUE)

add_executable_m7(HelloWorldProto
    hello_world_proto.cc
    ${PROTO_SRCS}
    ${PROTO_HDRS}
)

target_link_libraries(HelloWorldProto
    libs_base-m7_freertos
)

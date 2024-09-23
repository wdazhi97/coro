set(protobuf_DIR ../thirdparty/dist/lib64/cmake/protobuf)
set(absl_DIR ../thirdparty/dist/lib64/cmake/absl)
set(utf8_range_DIR ../thirdparty/dist/lib64/cmake/utf8_range)
set(gRPC_DIR ../thirdparty/dist/lib/cmake/grpc)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(Threads)

#
# Protobuf/Grpc source files
#
set(PROTO_FILES
    proto/address.proto
    proto/addressbook.proto
)

#
# Add Library target with protobuf sources
#
add_library(myproto ${PROTO_FILES})
target_link_libraries(myproto
    PUBLIC
        protobuf::libprotobuf
        gRPC::grpc
        gRPC::grpc++
)
target_include_directories(myproto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#
# Compile protobuf and grpc files in myproto target to cpp
#
get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET myproto LANGUAGE cpp PROTOC_OUT_DIR ./)
protobuf_generate(TARGET myproto LANGUAGE grpc GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc PROTOC_OUT_DIR ./ PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}")
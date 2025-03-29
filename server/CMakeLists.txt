cmake_minimum_required(VERSION 3.16)
project(rest_api_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Protobuf REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Set paths for protobuf compiler and gRPC plugin
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

# Proto file
get_filename_component(dt_proto "../proto_files/data_transfer.proto" ABSOLUTE)
get_filename_component(dt_proto_path "${dt_proto}" PATH)

# Generated sources
set(dt_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/data_transfer.pb.cc")
set(dt_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/data_transfer.pb.h")
set(dt_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/data_transfer.grpc.pb.cc")
set(dt_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/data_transfer.grpc.pb.h")

# Generate protocol buffers
add_custom_command(
      OUTPUT "${dt_proto_srcs}" "${dt_proto_hdrs}" "${dt_grpc_srcs}" "${dt_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${dt_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${dt_proto}"
      DEPENDS "${dt_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Create library from generated files
add_library(proto_lib
  ${dt_proto_srcs}
  ${dt_grpc_srcs})

# Link the library with necessary dependencies
target_link_libraries(proto_lib
  gRPC::grpc++
  protobuf::libprotobuf)

# Add the executable
add_executable(rest_api_server src/main.cpp)

# Link the executable with our proto library
target_link_libraries(rest_api_server
  proto_lib
  gRPC::grpc++
  protobuf::libprotobuf)
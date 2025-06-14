cmake_minimum_required(VERSION 3.10)
project(fdbus_hello)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set CMAKE_PREFIX_PATH to find the custom installation
set(CMAKE_PREFIX_PATH "/usr/local/cmake;${CMAKE_PREFIX_PATH}")

# Find packages
find_package(PkgConfig REQUIRED)
# Find the fdbus package
find_package(fdbus REQUIRED)
find_package(Protobuf REQUIRED)

# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)

# Set proto file path
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/HelloWorld.proto)
set(PROTO_SRC ${CMAKE_BINARY_DIR}/generated/HelloWorld.pb.cc)
set(PROTO_HDR ${CMAKE_BINARY_DIR}/generated/HelloWorld.pb.h)

# Custom command to generate proto files
add_custom_command(
  OUTPUT ${PROTO_SRC} ${PROTO_HDR}
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
  ARGS --cpp_out=${CMAKE_BINARY_DIR}/generated
       --proto_path=${CMAKE_SOURCE_DIR}/proto
       ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
  COMMENT "Generating protocol buffer files from ${PROTO_FILE}"
  VERBATIM
)

# Include directories
include_directories(
  ${FDBUS_INCLUDE_DIRS}
  ${Protobuf_INCLUDE_DIRS}
  ${CMAKE_BINARY_DIR}/generated  # For generated protobuf files
)

add_executable(fdbxHelloServer
    ${CMAKE_SOURCE_DIR}/src/HelloServer.cpp ${PROTO_SRC})

add_executable(fdbxHelloClient
    ${CMAKE_SOURCE_DIR}/src/HelloClient.cpp ${PROTO_SRC})

# Link against libraries
target_link_libraries(fdbxHelloServer
  fdbus fdbus-clib
  ${Protobuf_LIBRARIES}
)

# Link against libraries
target_link_libraries(fdbxHelloClient
  fdbus fdbus-clib
  ${Protobuf_LIBRARIES}
)
cmake_minimum_required(VERSION 3.10)
project(fdbus_hello)

# Set find_package(Protobuf REQUIRED)C++ standard
set(CMAKE_CXX_STANDARD 14)

# Set CMAKE_PREFIX_PATH to find the custom installation
list(APPEND CMAKE_PREFIX_PATH "/usr/local")

# Find packages
find_package(PkgConfig REQUIRED)
# Find the fdbus package
find_package(fdbus REQUIRED)
find_package(Protobuf REQUIRED)
if(NOT Protobuf_PROTOC_EXECUTABLE)
  find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc)
  if(NOT Protobuf_PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Could not find protoc executable. Please ensure Protobuf is installed and CMake can find it.")
  endif()
endif()
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
include_directories(${Protobuf_INCLUDE_DIRS})
# Create directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/generated)

# Set proto file path

# Proto file paths
set(PROTO_FILE ${CMAKE_SOURCE_DIR}/proto/HelloWorld.proto)
set(PROTO_SRC ${CMAKE_BINARY_DIR}/generated/HelloWorld.pb.cc)
set(PROTO_HDR ${CMAKE_BINARY_DIR}/generated/HelloWorld.pb.h)

# Custom command to generate proto files
add_custom_command(
  OUTPUT ${PROTO_SRC} ${PROTO_HDR}
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/generated
  COMMAND ${CMAKE_COMMAND} -E echo "Running: ${Protobuf_PROTOC_EXECUTABLE} --proto_path=${CMAKE_SOURCE_DIR}/proto --cpp_out=${CMAKE_BINARY_DIR}/generated ${PROTO_FILE}"
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
          --proto_path=${CMAKE_SOURCE_DIR}/proto
          --cpp_out=${CMAKE_BINARY_DIR}/generated
          ${PROTO_FILE}
  DEPENDS ${PROTO_FILE}
  COMMENT "Generating protocol buffer files from ${PROTO_FILE}"
  VERBATIM
)

# Custom target to generate all protos
add_custom_target(generate_protos ALL DEPENDS ${PROTO_SRC} ${PROTO_HDR})

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

# Ensure proto files are generated before building executables
add_dependencies(fdbxHelloServer generate_protos)
add_dependencies(fdbxHelloClient generate_protos)

# Link against libraries
target_link_libraries(fdbxHelloServer PRIVATE
  fdbus fdbus-clib
  protobuf::libprotobuf
)

# Link against libraries
target_link_libraries(fdbxHelloClient PRIVATE
  fdbus fdbus-clib
  protobuf::libprotobuf
)
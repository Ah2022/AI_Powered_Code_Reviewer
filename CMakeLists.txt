cmake_minimum_required(VERSION 3.10)
project(cpp_code_reviewer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM and Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Find libcurl
find_package(CURL REQUIRED)

# Include directories
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add definitions
add_definitions(${LLVM_DEFINITIONS})

# Source files
set(SOURCES
    src/main.cpp
    src/clang_parser.cpp
    src/code_analyzer.cpp
    src/llm_client.cpp
    src/utils.cpp
)

# Create executable
add_executable(cpp_code_reviewer ${SOURCES})

# Link libraries
target_link_libraries(cpp_code_reviewer
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
    clangTooling
    ${CURL_LIBRARIES}
)

# Install target
install(TARGETS cpp_code_reviewer DESTINATION bin)

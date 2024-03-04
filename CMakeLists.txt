cmake_minimum_required(VERSION 3.22)
project(LLVMProj)

find_package(LLVM CONFIG REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message("LLVM STATUS:
    Definitions: ${LLVM_DEFINITIONS}
    Include dirs: ${LLVM_INCLUDE_DIRS}
    Libraries: ${LLVM_LIBRARIES}
    Tools: ${LLVM_TOOLS_BINARY_DIR}"
)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=undefined")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(core)

set(CMAKE_CXX_STANDARD 23)
file(GLOB_RECURSE FILES "*.cpp" "*.h")
list(FILTER FILES EXCLUDE REGEX "cmake-build.*")
#list(FILTER FILES EXCLUDE REGEX "llvm-gen/.*")

message("Files: ${FILES}")

add_executable(LLVMProj ${FILES}
        llvm-gen/operators.cpp
        llvm-gen/operators.h)

target_link_libraries(LLVMProj PRIVATE LLVMCore)

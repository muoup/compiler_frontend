cmake_minimum_required(VERSION 3.22)
project(llvm_frontend)

find_package(LLVM CONFIG REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message("LLVM STATUS:
    Definitions: ${LLVM_DEFINITIONS}
    Include Dirs: ${LLVM_INCLUDE_DIRS}
    Libraries: ${LLVM_LIBRARIES}
    Tools: ${LLVM_TOOLS_BINARY_DIR}"
)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(core)

set(CMAKE_CXX_STANDARD 23)
file(GLOB_RECURSE FILES "*.cpp" "*.h")
list(FILTER FILES EXCLUDE REGEX "cmake-build.*")

add_executable(llvm_frontend ${FILES}
        ast/parser_methods/expression.cpp
        ast/parser_methods/expression.h)

target_link_libraries(llvm_frontend PRIVATE LLVMCore)

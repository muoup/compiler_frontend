cmake_minimum_required(VERSION 3.22)
project(llvm_frontend)

# Since LLVM is a rather large dependency, it can be disabled for
# non-codegen development on machines which do not have LLVM installed.
# For all master branch builds, LLVM should be enabled by default.
option(ENABLE_LLVM "Build with LLVM support" ON)

# Testing is enabled by default, but can be disabled for faster builds.
# For all master branch builds, interface should be disabled by default.
option(ENABLE_TESTING "Build with testing support" ON)

set(CMAKE_CXX_STANDARD 23)
set(FILES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
        interface/file_reader.cpp
        interface/file_reader.h
        interface/argument_parser.cpp
        interface/argument_parser.h)

function(build_dir dir)
#    message("${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h")
    file(GLOB_RECURSE FILES_TEMP "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h")
    set(FILES ${FILES} ${FILES_TEMP} PARENT_SCOPE)
endfunction()

message("Building...")

build_dir("ast")
build_dir("lexer")
build_dir("interface")

if (ENABLE_LLVM)
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

    build_dir("llvm")
endif ()

if (ENABLE_TESTING)
    # Include the test scripts in the /test_code/ directory
    file(GLOB_RECURSE TEMP_FILES "interface/test_code/*")

    # Add temp files to build files
    foreach(TEMP_FILE ${TEMP_FILES})
        file(COPY ${TEMP_FILE} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test_code")
    endforeach ()
endif ()

message("FILES: ${FILES}")
add_executable(llvm_frontend ${FILES})

if (ENABLE_LLVM)
    target_link_libraries(llvm_frontend PRIVATE ${LLVM_LIBRARIES})
endif ()

message("BUILD END")
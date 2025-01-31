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
set(FILES "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")

function(build_dir dir)
    file(GLOB_RECURSE FILES_TEMP "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/*.h")
    set(FILES ${FILES} ${FILES_TEMP} PARENT_SCOPE)
endfunction()

message(STATUS "Building...")

# Copy the lib directory into the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/lib DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

build_dir("ast")
build_dir("lexer")
build_dir("interface")
build_dir("preprocess")

if (ENABLE_LLVM)
    find_package(LLVM CONFIG QUIET)

    if (NOT LLVM_FOUND)
        message(STATUS "LLVM NOT FOUND, DISABLING LLVM")
        set(ENABLE_LLVM OFF)
    else()
        message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
        message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

        message(STATUS "LLVM STATUS:
            Definitions: ${LLVM_DEFINITIONS}
            Include Dirs: ${LLVM_INCLUDE_DIRS}
            Libraries: ${LLVM_LIBRARIES}
            Tools: ${LLVM_TOOLS_BINARY_DIR}"
        )

        include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
        add_definitions(${LLVM_DEFINITIONS})

        add_compile_definitions(ENABLE_LLVM)

        llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD} support core irreader codegen mc mcparser option)

        build_dir("llvm-gen")
    endif()
endif ()

add_executable(llvm_frontend ${FILES})

if (ENABLE_LLVM)
    target_link_libraries(llvm_frontend PRIVATE ${llvm_libs})
    message(STATUS "LLVM ENABLED")
else()
#    target_link_libraries(llvm_frontend PRIVATE compiler_backend)
#    message(STATUS "compiler_backend ENABLED")
endif()

message(STATUS "BUILD END")
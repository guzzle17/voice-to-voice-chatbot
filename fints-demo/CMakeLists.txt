cmake_minimum_required(VERSION 3.10)
project(fints-demo CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Find llama.cpp
set(LLAMA_CPP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/llama.cpp")
if(NOT EXISTS "${LLAMA_CPP_DIR}")
    message(FATAL_ERROR "llama.cpp not found. Run scripts/setup.sh first")
endif()

# Include directories
include_directories(
    ${LLAMA_CPP_DIR}/include
    ${LLAMA_CPP_DIR}/ggml/include
    ${LLAMA_CPP_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link directories
link_directories(
    ${LLAMA_CPP_DIR}/build/src
    ${LLAMA_CPP_DIR}/build/ggml/src
    ${LLAMA_CPP_DIR}/build/ggml/src/ggml-metal
    ${LLAMA_CPP_DIR}/build/ggml/src/ggml-blas
)

# Platform-specific settings
if(APPLE)
    find_library(ACCELERATE_FRAMEWORK Accelerate)
    find_library(METAL_FRAMEWORK Metal)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    set(PLATFORM_LIBS ${ACCELERATE_FRAMEWORK} ${METAL_FRAMEWORK} ${FOUNDATION_FRAMEWORK})
elseif(UNIX)
    set(PLATFORM_LIBS)
endif()

# FINTs library
add_library(fints STATIC
    src/fints.cpp
)

target_link_libraries(fints
    llama
    ggml
    ${PLATFORM_LIBS}
    pthread
)

# Basic demo executable
add_executable(basic_demo
    examples/basic_demo.cpp
)

target_link_libraries(basic_demo
    fints
    llama
    ggml
    ggml-base
    ggml-cpu
    ggml-metal
    ggml-blas
    ${PLATFORM_LIBS}
    pthread
)

# Set RPATH for macOS/Linux
if(APPLE)
    set_target_properties(basic_demo PROPERTIES
        BUILD_RPATH "${LLAMA_CPP_DIR}/build/bin"
        INSTALL_RPATH "${LLAMA_CPP_DIR}/build/bin"
    )
elseif(UNIX)
    set_target_properties(basic_demo PROPERTIES
        BUILD_RPATH "${LLAMA_CPP_DIR}/build/bin"
        INSTALL_RPATH "$ORIGIN/../llama.cpp/build/bin"
    )
endif()

# LaMP demo executable (requires nlohmann-json)
find_package(nlohmann_json 3.2.0 QUIET)
if(nlohmann_json_FOUND)
    add_executable(lamp_demo
        examples/lamp_demo.cpp
    )
    
    target_link_libraries(lamp_demo
        fints
        llama
        ggml
        ggml-base
        ggml-cpu
        ggml-metal
        ggml-blas
        nlohmann_json::nlohmann_json
        ${PLATFORM_LIBS}
        pthread
    )
    
    if(APPLE)
        set_target_properties(lamp_demo PROPERTIES
            BUILD_RPATH "${LLAMA_CPP_DIR}/build/bin"
            INSTALL_RPATH "${LLAMA_CPP_DIR}/build/bin"
        )
    elseif(UNIX)
        set_target_properties(lamp_demo PROPERTIES
            BUILD_RPATH "${LLAMA_CPP_DIR}/build/bin"
            INSTALL_RPATH "$ORIGIN/../llama.cpp/build/bin"
        )
    endif()
    
    message(STATUS "LaMP demo will be built (nlohmann-json found)")
else()
    message(STATUS "LaMP demo skipped (install nlohmann-json: brew install nlohmann-json)")
endif()

# Install targets
install(TARGETS basic_demo DESTINATION bin)
install(FILES src/fints.h DESTINATION include)

# Print configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "llama.cpp: ${LLAMA_CPP_DIR}")

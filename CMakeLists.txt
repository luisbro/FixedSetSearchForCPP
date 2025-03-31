cmake_minimum_required(VERSION 3.12)
project(fixed_set_search)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Optimization flags for different build types
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -march=native")

# Include directories
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include;${CMAKE_SOURCE_DIR}/include/partition-comparison")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

# Source files
file(GLOB SOURCES "${SRC_DIR}/*.cpp")

# Define executable target
add_executable(${PROJECT_NAME} ${SOURCES})

# Link include directories to target
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

# Set output binary name
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "fixed_set_search_exe")

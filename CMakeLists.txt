# Copyright 2018 WolkAbout Technology s.r.o.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.5)
project(WolkGatewayModule C CXX)

set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}    -Wall -Wextra -pedantic                                                    \
                                            -pedantic-errors -Wcast-align                                              \
                                            -Wcast-qual -Wconversion                                                   \
                                            -Wdisabled-optimization                                                    \
                                            -Wfloat-equal -Wformat=2                                                   \
                                            -Werror=init-self                                                          \
                                            -Werror=missing-field-initializers -Wmissing-format-attribute              \
                                            -Wmissing-include-dirs -Wmissing-noreturn                                  \
                                            -Werror=pointer-arith                                                      \
                                            -Wno-packed  -Wno-padded -Wredundant-decls                                 \
                                            -Werror=shadow -Werror=stack-protector                                     \
                                            -Wstrict-aliasing=2                                                        \
                                            -Werror=unreachable-code -Wno-unused                                       \
                                            -Wvariadic-macros                                                          \
                                            -Wwrite-strings -Werror=non-virtual-dtor")

# library output
if (NOT DEFINED CMAKE_LIBRARY_INCLUDE_DIRECTORY)
    set(CMAKE_LIBRARY_INCLUDE_DIRECTORY ${CMAKE_BINARY_DIR}/include)
endif ()

if (NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif ()

if (NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
endif ()

link_directories(${CMAKE_BINARY_DIR}/lib)

find_package(Threads REQUIRED)

# WolkAbout c++ SDK
set(BUILD_CONNECTIVITY ON CACHE BOOL "Build the library with Paho MQTT and allow MQTT connection to the platform.")
set(BUILD_POCO OFF CACHE BOOL "Build the library with Poco.")
set(BUILD_GTEST ON CACHE BOOL "Build the library with GTest.")
set(BUILD_AWS_LOG_UPLOADER OFF CACHE BOOL "Build the library with AwsLogUploader.")
add_subdirectory(WolkSDK-Cpp)

# WolkAbout c++ Connector
include_directories("src")

file(GLOB_RECURSE HEADER_FILES "src/*.h" "src/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} WolkAboutCore Threads::Threads)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")

# Tests
include_directories("tests")

file(GLOB_RECURSE TESTS_HEADER_FILES "tests/*.h" "tests/*.hpp")
file(GLOB_RECURSE TESTS_SOURCE_FILES "tests/*.cpp")

add_executable(${PROJECT_NAME}Tests ${TESTS_SOURCE_FILES})
target_include_directories(${PROJECT_NAME}Tests PUBLIC ${CMAKE_LIBRARY_INCLUDE_DIRECTORY})
target_link_libraries(${PROJECT_NAME}Tests ${PROJECT_NAME} gtest_main gmock_main gmock gtest)
set_target_properties(${PROJECT_NAME}Tests PROPERTIES INSTALL_RPATH "$ORIGIN/lib")
set_target_properties(${PROJECT_NAME}Tests PROPERTIES EXCLUDE_FROM_ALL TRUE)
add_dependencies(${PROJECT_NAME}Tests libgtest)

add_custom_target(tests ${PROJECT_NAME}Tests deviceConfiguration.json)

# Example
include_directories("example")

file(GLOB_RECURSE EXAMPLE_HEADER_FILES "example/*.h")
file(GLOB_RECURSE EXAMPLE_SOURCE_FILES "example/*.cpp")

add_executable(${PROJECT_NAME}Example ${EXAMPLE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}Example ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME}Example PROPERTIES INSTALL_RPATH "$ORIGIN/lib")

add_subdirectory(cmake)

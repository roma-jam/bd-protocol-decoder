## SPDX-FileCopyrightText: 2026 Roman Leonov
## SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20)

project(stream_parser
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# DECODER
add_library(stream_parser_lib
    src/decoder.cpp
)

target_include_directories(stream_parser_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(stream_parser_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# EXECUTABLE
add_executable(stream_parser_app
    src/main.cpp
)

target_link_libraries(stream_parser_app
    PRIVATE
        stream_parser_lib
)

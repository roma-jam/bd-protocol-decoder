## SPDX-FileCopyrightText: 2026 Roman Leonov
## SPDX-License-Identifier: Apache-2.0

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(decoder_tests
    decoder_tests.cpp
)

target_link_libraries(decoder_tests
    PRIVATE
        stream_parser_lib
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(decoder_tests)

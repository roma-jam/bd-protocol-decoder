#!/usr/bin/env bash

set -e

BUILD_DIR="build"
EXEC_NAME="protocol_decoder"

cmake -S . -B "${BUILD_DIR}" -DBUILD_TESTS=OFF
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "Build complete: ${BUILD_DIR}/stream_parser_app"

yes | cp -rf ${BUILD_DIR}/stream_parser_app ${EXEC_NAME}

echo " "
echo "To decode example input, run: "
echo "    ./${EXEC_NAME} \"1111 1111 0101 1000 1101 0010 0001 0100 0010 0101 1111 1111 1111 1111 1111 1111 0101 1000 1101 1110 0001 0100 0010 1101 1111 1111 1111 0101 1000 1101 1110 0001 0100 0010 1001 1111 1111 1111 1111\""

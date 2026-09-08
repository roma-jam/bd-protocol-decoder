#!/usr/bin/env bash

set -e

BUILD_DIR="build"
EXEC_NAME="protocol_decoder"

cmake -S . -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" -j"$(nproc)"

echo "Build complete: ${BUILD_DIR}/stream_parser_app"

yes | cp -rf ${BUILD_DIR}/stream_parser_app ${EXEC_NAME}

echo " "
echo "To decode, run: "
echo "    ./${EXEC_NAME} <string_to_decode>"

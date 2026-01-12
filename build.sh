#!/bin/bash

BUILD_TYPE=${1:-Debug}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT_DIR="$SCRIPT_DIR/bin/$BUILD_TYPE"

mkdir -p "$BUILD_DIR" "$OUTPUT_DIR"

pushd "$BUILD_DIR" > /dev/null

echo "Building LumX ($BUILD_TYPE)..."

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$BUILD_TYPE "$SCRIPT_DIR"
if [ $? -ne 0 ]; then
    echo "CMake configuration failed"
    popd > /dev/null
    exit 1
fi

cmake --build . --config $BUILD_TYPE --parallel 4
if [ $? -ne 0 ]; then
    echo "Build failed"
    popd > /dev/null
    exit 1
fi

echo "Build completed successfully"
echo "Output: $OUTPUT_DIR"

popd > /dev/null

if [ "$BUILD_TYPE" = "Release" ]; then
    echo ""
    echo "Creating Release package..."
    mkdir -p "$SCRIPT_DIR/dist"
    cp "$OUTPUT_DIR/LumX" "$SCRIPT_DIR/dist/" 2>/dev/null || cp "$OUTPUT_DIR/LumX.exe" "$SCRIPT_DIR/dist/"
fi

#!/bin/bash

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "Messenger Build Script"
echo "========================================"
echo ""

CONFIG="${1:-Release}"
echo -e "${YELLOW}Build configuration: ${CONFIG}${NC}"
echo ""

BUILD_DIR="build"

if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi

if [ -d "bin" ]; then
    echo "Removing bin directory..."
    rm -rf bin
fi

echo "Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "Detecting available generator..."

GENERATOR=""
EXTRA_ARGS=""

if command -v ninja &> /dev/null; then
    echo "Found Ninja"
    GENERATOR="Ninja"
elif command -v make &> /dev/null; then
    echo "Found Make"
    GENERATOR="Unix Makefiles"
else
    echo -e "${RED}No suitable generator found!${NC}"
    exit 1
fi

echo "Using generator: $GENERATOR"
echo ""

echo "Configuring CMake..."
cmake .. -G "$GENERATOR" -DCMAKE_BUILD_TYPE="$CONFIG" $EXTRA_ARGS

echo ""
echo "Building..."
cmake --build . -- -j$(nproc)

echo ""
echo "========================================"
echo -e "${GREEN}Build completed successfully!${NC}"
echo "========================================"
echo ""
echo "Output location:"
echo "  bin/${CONFIG}/x64/Client/Client"
echo "  bin/${CONFIG}/x64/Server/Server"
echo "  bin/${CONFIG}/x64/lib/libCommon.a"
echo ""

cd ..
CONFIG="${1:-Release}"

echo "========================================"
echo "Messenger Build Script"
echo "========================================"
echo ""
echo "Build configuration: $CONFIG"
echo ""

rm -rf build bin
mkdir build && cd build

if command -v ninja &> /dev/null; then
    echo "Using Ninja generator"
    cmake .. -GNinja -DCMAKE_BUILD_TYPE="$CONFIG"
    cmake --build .
else
    echo "Using Make generator"
    cmake .. -G"Unix Makefiles" -DCMAKE_BUILD_TYPE="$CONFIG"
    cmake --build . -- -j$(nproc)
fi

cd ..
echo ""
echo "Build complete!"
if [ "$CONFIG" = "Release" ]; then
    echo "Run server: ./bin/Release/Server/Server"
    echo "Run client: ./bin/Release/Client/Client"
else
    echo "Run server: ./bin/Debug/Server/Server"
    echo "Run client: ./bin/Debug/Client/Client"
fi
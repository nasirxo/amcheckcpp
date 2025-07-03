#!/bin/bash

# Quick build script for CentOS 7 with old CMake
# This script handles the CMake version issue and CUDA problems

echo "Quick build for CentOS 7 with old CMake..."

# Check if cmake3 is available
if command -v cmake3 &> /dev/null; then
    CMAKE_CMD="cmake3"
    echo "✅ Using cmake3"
elif command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
    if [[ $(echo "$CMAKE_VERSION 3.5" | awk '{print ($1 >= $2)}') == 1 ]]; then
        CMAKE_CMD="cmake"
        echo "✅ Using cmake $CMAKE_VERSION"
    else
        echo "❌ CMake version $CMAKE_VERSION is too old (need 3.5+)"
        echo "Installing cmake3..."
        sudo yum install -y cmake3
        CMAKE_CMD="cmake3"
    fi
else
    echo "❌ No CMake found, installing cmake3..."
    sudo yum install -y cmake3
    CMAKE_CMD="cmake3"
fi

# Create build directory
mkdir -p build
cd build

echo "Configuring with $CMAKE_CMD..."

# Configure with CUDA disabled for safety on older systems
$CMAKE_CMD .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_CUDA=OFF \
    -DBUILD_STATIC=ON

if [ $? -ne 0 ]; then
    echo "❌ Configuration failed"
    exit 1
fi

echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Executable: $(pwd)/bin/amcheck"
    
    # Test the binary
    if [ -f "bin/amcheck" ]; then
        echo "Testing binary..."
        ./bin/amcheck --version
    fi
else
    echo "❌ Build failed"
    exit 1
fi

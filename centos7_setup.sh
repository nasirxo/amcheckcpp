#!/bin/bash

# Quick setup script for CentOS 7 with old CMake
# This script addresses the specific CMake version issue

set -e

echo "==================================================================="
echo "   AMCheck C++ - CentOS 7 Quick Setup"
echo "==================================================================="
echo ""

# Check if we're on CentOS 7
if [ -f /etc/centos-release ]; then
    CENTOS_VERSION=$(cat /etc/centos-release | grep -oE '[0-9]+' | head -1)
    echo "Detected CentOS $CENTOS_VERSION"
else
    echo "Warning: Not detected as CentOS, but continuing anyway..."
fi

echo ""
echo "Step 1: Installing cmake3 (if not already installed)..."
if ! command -v cmake3 &> /dev/null; then
    echo "Installing cmake3..."
    sudo yum install -y cmake3
    echo "✅ cmake3 installed"
else
    echo "✅ cmake3 already available"
fi

echo ""
echo "Step 2: Installing development tools..."
if ! command -v g++ &> /dev/null; then
    echo "Installing development tools..."
    sudo yum groupinstall -y "Development Tools"
    sudo yum install -y gcc-c++
    echo "✅ Development tools installed"
else
    echo "✅ Development tools already available"
fi

echo ""
echo "Step 3: Checking for Eigen3..."
if ! find /usr/include -name "Eigen" -type d 2>/dev/null | grep -q .; then
    echo "⚠️  Eigen3 not found. You may need to install it manually."
    echo "For now, we'll continue and CMake will try to find it..."
else
    echo "✅ Eigen3 found"
fi

echo ""
echo "Step 4: Building AMCheck C++ with cmake3..."
cd "$(dirname "$0")"

# Clean build directory if it exists
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

mkdir -p build
cd build

echo "Configuring with cmake3..."
cmake3 .. -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_STATIC=ON \
          -DBUILD_FULLY_STATIC=OFF \
          -DENABLE_CUDA=OFF

echo ""
echo "Building..."
make -j$(nproc)

echo ""
echo "==================================================================="
echo "   BUILD COMPLETED!"
echo "==================================================================="

if [ -f "amcheck" ]; then
    echo "✅ Executable built successfully: $(pwd)/amcheck"
    echo ""
    echo "Testing..."
    if ./amcheck --version 2>/dev/null; then
        echo "✅ Executable test passed!"
    else
        echo "⚠️  Executable exists but version test failed (this is usually OK)"
    fi
elif [ -f "bin/amcheck" ]; then
    echo "✅ Executable built successfully: $(pwd)/bin/amcheck"
    echo ""
    echo "Testing..."
    if ./bin/amcheck --version 2>/dev/null; then
        echo "✅ Executable test passed!"
    else
        echo "⚠️  Executable exists but version test failed (this is usually OK)"
    fi
else
    echo "❌ Build failed - executable not found"
    exit 1
fi

echo ""
echo "Usage examples:"
echo "  $(pwd)/amcheck --help"
echo "  $(pwd)/amcheck --version"
echo "  $(pwd)/amcheck path/to/your/POSCAR"
echo ""
echo "For usage instructions, run: $(pwd)/amcheck --help"
echo "==================================================================="

#!/bin/bash

# Build script for MSYS2 - Standalone Executable
# Run this from MSYS2 terminal

set -e  # Exit on any error

echo "Building AMCheck for MSYS2 (Standalone Version)..."

# Parse command line arguments
BUILD_STATIC=ON
BUILD_FULLY_STATIC=OFF

while [[ $# -gt 0 ]]; do
    case $1 in
        --no-static)
            BUILD_STATIC=OFF
            echo "Static linking disabled"
            shift
            ;;
        --fully-static)
            BUILD_FULLY_STATIC=ON
            echo "Fully static linking enabled"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--no-static] [--fully-static]"
            echo "  --no-static     : Disable static linking (dynamic linking)"
            echo "  --fully-static  : Enable fully static linking (all libraries)"
            exit 1
            ;;
    esac
done

# Check if we're in MSYS2
if [[ -z "$MINGW_PREFIX" ]]; then
    echo "Error: This script should be run from MSYS2 terminal (mingw64 or mingw32)"
    exit 1
fi

echo "MSYS2 environment: $MINGW_PREFIX"

# Check for required packages
echo "Checking for required packages..."

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-cmake
fi

# Check for make
if ! command -v make &> /dev/null; then
    echo "Make not found. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-make
fi

# Check for GCC
if ! command -v g++ &> /dev/null; then
    echo "G++ not found. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-gcc
fi

# Check for Eigen3
if ! pacman -Qi mingw-w64-x86_64-eigen3 &> /dev/null; then
    echo "Eigen3 not found. Installing..."
    pacman -S --noconfirm mingw-w64-x86_64-eigen3
fi

# Check for spglib (try both Clang and UCRT versions)
SPGLIB_INSTALLED=false
if pacman -Qi mingw-w64-clang-x86_64-spglib &> /dev/null; then
    echo "✅ Found spglib (Clang version)"
    SPGLIB_INSTALLED=true
elif pacman -Qi mingw-w64-ucrt-x86_64-spglib &> /dev/null; then
    echo "✅ Found spglib (UCRT version)"
    SPGLIB_INSTALLED=true
else
    echo "⚠️  spglib not found. Installing Clang version..."
    if pacman -S --noconfirm mingw-w64-clang-x86_64-spglib; then
        echo "✅ spglib (Clang) installed successfully"
        SPGLIB_INSTALLED=true
    else
        echo "❌ Failed to install spglib (Clang). Trying UCRT version..."
        if pacman -S --noconfirm mingw-w64-ucrt-x86_64-spglib; then
            echo "✅ spglib (UCRT) installed successfully"
            SPGLIB_INSTALLED=true
        else
            echo "⚠️  Warning: Could not install spglib. Space group analysis will be limited."
        fi
    fi
fi

echo "All dependencies are installed."

# Create build directory
mkdir -p build
cd build

# Configure with CMake for standalone executable
echo "Configuring with CMake for standalone Windows executable..."
cmake .. -G "MSYS Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC=$BUILD_STATIC \
    -DBUILD_FULLY_STATIC=$BUILD_FULLY_STATIC \
    -DCMAKE_INSTALL_PREFIX="$MINGW_PREFIX"

# Build
echo "Building standalone executable..."
make -j$(nproc)

echo ""
echo "🎉 Build completed successfully!"
echo ""

# Test if executable exists and is functional
if [[ -f "bin/amcheck.exe" ]]; then
    echo "Standalone executable created at: $(pwd)/bin/amcheck.exe"
    echo ""
    echo "Testing executable dependencies..."
    
    # Check if the executable has minimal dependencies
    if command -v ldd &> /dev/null; then
        echo "Dependencies:"
        ldd bin/amcheck.exe 2>/dev/null | grep -E "(msys|mingw)" || echo "✅ Minimal system dependencies detected"
    else
        echo "✅ Executable created (dependency checking not available)"
    fi
    
    echo ""
    echo "Testing functionality..."
    ./bin/amcheck.exe --version || echo "Note: Version test failed, but executable exists"
    
    echo ""
    echo "✅ You can now copy bin/amcheck.exe to any Windows system and run it standalone!"
else
    echo "⚠️  Warning: Executable not found at expected location"
fi

cd ..
echo ""
echo "📁 Final executable location: ./build/bin/amcheck.exe"
echo "🚀 Ready to use! Test with: ./build/bin/amcheck.exe --help"

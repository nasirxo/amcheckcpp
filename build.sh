#!/bin/bash

set -e  # Exit on any error

echo "Building AMCheck C++ (Standalone Version)..."

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

# Function to detect OS
detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        echo "linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
        echo "windows"
    else
        echo "unknown"
    fi
}

OS=$(detect_os)
echo "Detected OS: $OS"

# Clean build directory if switching between different environments
if [ -f "build/CMakeCache.txt" ]; then
    # Check if we're switching between Windows and Linux paths
    if grep -q "F:/" build/CMakeCache.txt && [[ "$OS" == "linux" ]]; then
        echo "Detected Windows->Linux switch, cleaning build directory..."
        rm -rf build
    elif grep -q "/mnt/" build/CMakeCache.txt && [[ "$OS" == "windows" ]]; then
        echo "Detected Linux->Windows switch, cleaning build directory..."
        rm -rf build
    fi
fi

# Create build directory
mkdir -p build
cd build

# Check for dependencies based on OS
echo "Checking dependencies for $OS..."

case $OS in
    "linux")
        # Check for essential tools
        if ! command -v cmake &> /dev/null; then
            echo "CMake not found. Please install it:"
            echo "  Ubuntu/Debian: sudo apt-get install cmake"
            echo "  Fedora: sudo dnf install cmake"
            exit 1
        fi
        
        if ! command -v g++ &> /dev/null; then
            echo "G++ not found. Please install it:"
            echo "  Ubuntu/Debian: sudo apt-get install build-essential"
            echo "  Fedora: sudo dnf install gcc-c++"
            exit 1
        fi
        
        # Check for Eigen3
        if ! pkg-config --exists eigen3 2>/dev/null && ! find /usr/include -name "Eigen" -type d 2>/dev/null | grep -q .; then
            echo "Eigen3 not found. Please install it:"
            echo "  Ubuntu/Debian: sudo apt-get install libeigen3-dev"
            echo "  Fedora: sudo dnf install eigen3-devel"
            exit 1
        fi
        
        # Check for spglib (optional but recommended)
        SPGLIB_FOUND=false
        
        # Check various ways spglib might be installed
        if pkg-config --exists symspg 2>/dev/null; then
            echo "✅ Found spglib via pkg-config (symspg)"
            SPGLIB_FOUND=true
        elif pkg-config --exists spglib 2>/dev/null; then
            echo "✅ Found spglib via pkg-config (spglib)"
            SPGLIB_FOUND=true
        elif find /usr/include -name "spglib.h" 2>/dev/null | grep -q .; then
            echo "✅ Found spglib header at: $(find /usr/include -name "spglib.h" 2>/dev/null | head -1)"
            # Check for library files
            if find /usr/lib* /lib* -name "libsymspg*" 2>/dev/null | grep -q . || \
               find /usr/lib* /lib* -name "libspg*" 2>/dev/null | grep -q . || \
               ldconfig -p 2>/dev/null | grep -q "symspg\|spglib"; then
                echo "✅ Found spglib library files"
                SPGLIB_FOUND=true
            fi
        fi
        
        if [ "$SPGLIB_FOUND" = false ]; then
            echo "⚠️  Warning: spglib not found. Space group analysis will be limited."
            echo "To install spglib:"
            echo "  Ubuntu/Debian: sudo apt-get install libsymspg-dev"
            echo "  Fedora: sudo dnf install spglib-devel"
            echo "Continuing without spglib..."
        else
            echo "🎉 spglib found - full space group analysis available!"
        fi
        ;;
        
    "macos")
        if ! command -v cmake &> /dev/null; then
            echo "CMake not found. Please install it:"
            echo "  brew install cmake"
            exit 1
        fi
        
        if ! command -v g++ &> /dev/null; then
            echo "G++ not found. Please install Xcode command line tools:"
            echo "  xcode-select --install"
            exit 1
        fi
        ;;
esac

echo "Configuring with CMake for standalone executable..."
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_STATIC=$BUILD_STATIC \
         -DBUILD_FULLY_STATIC=$BUILD_FULLY_STATIC

echo "Building..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo "Build completed successfully!"

# Determine executable name based on OS
if [[ "$OS" == "windows" ]]; then
    EXE_NAME="amcheck.exe"
else
    EXE_NAME="amcheck"
fi

if [ -f "bin/$EXE_NAME" ]; then
    echo "Executable location: $(pwd)/bin/$EXE_NAME"
    echo "Testing executable..."
    ./bin/$EXE_NAME --version 2>/dev/null || echo "Note: Version test failed, but executable exists"
else
    echo "Executable location: $(pwd)/$EXE_NAME"
    if [ -f "$EXE_NAME" ]; then
        echo "Testing executable..."
        ./$EXE_NAME --version 2>/dev/null || echo "Note: Version test failed, but executable exists"
    fi
fi

cd ..
echo "Done! You can run the program with: ./build/bin/$EXE_NAME or ./build/$EXE_NAME"

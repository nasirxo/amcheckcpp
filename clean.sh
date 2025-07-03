#!/bin/bash

# Clean build script - removes build artifacts
# Useful when switching between different environments

echo "Cleaning build directory..."

if [ -d "build" ]; then
    rm -rf build
    echo "✅ Build directory removed"
else
    echo "ℹ️  Build directory doesn't exist"
fi

# Also clean any object files that might be lying around
find . -name "*.o" -delete 2>/dev/null || true
find . -name "*.obj" -delete 2>/dev/null || true
find . -name "CMakeCache.txt" -delete 2>/dev/null || true

echo "✅ Clean completed"
echo "You can now run ./build.sh or ./build_msys2.sh to rebuild"

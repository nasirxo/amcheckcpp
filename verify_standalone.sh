#!/bin/bash

# Standalone Binary Verification Script
# Tests if the AMCheck executable is truly standalone

set -e

echo "========================================================"
echo "         AMCheck Standalone Binary Verification"
echo "========================================================"

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    EXE_NAME="amcheck"
    DEPENDENCY_CMD="ldd"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    EXE_NAME="amcheck"
    DEPENDENCY_CMD="otool -L"
elif [[ "$OSTYPE" == "msys"* ]] || [[ "$OSTYPE" == "cygwin"* ]]; then
    PLATFORM="windows"
    EXE_NAME="amcheck.exe"
    DEPENDENCY_CMD="ldd"
else
    PLATFORM="unknown"
    EXE_NAME="amcheck"
    DEPENDENCY_CMD="ldd"
fi

echo "Platform detected: $PLATFORM"
echo "Executable name: $EXE_NAME"
echo ""

# Check if executable exists
EXE_PATH="build/bin/$EXE_NAME"
if [[ ! -f "$EXE_PATH" ]]; then
    echo "❌ Error: Executable not found at $EXE_PATH"
    echo "Please build the project first with ./build.sh or ./build_msys2.sh"
    exit 1
fi

echo "✅ Executable found: $EXE_PATH"

# Check file size
FILE_SIZE=$(ls -lh "$EXE_PATH" | awk '{print $5}')
echo "📦 File size: $FILE_SIZE"

# Check if it's executable
if [[ -x "$EXE_PATH" ]]; then
    echo "✅ File is executable"
else
    echo "❌ File is not executable"
    exit 1
fi

echo ""
echo "🔍 Checking dependencies..."

# Check dependencies
if command -v $DEPENDENCY_CMD &> /dev/null; then
    echo "Dependencies found:"
    echo "----------------------------------------"
    
    if [[ "$PLATFORM" == "windows" ]]; then
        # For Windows, check for problematic MSYS2/MinGW dependencies
        DEPS=$($DEPENDENCY_CMD "$EXE_PATH" 2>/dev/null)
        echo "$DEPS"
        
        if echo "$DEPS" | grep -qi "msys\|mingw"; then
            echo ""
            echo "⚠️  Warning: MSYS2/MinGW dependencies detected!"
            echo "This executable may not be fully standalone."
            echo "Consider rebuilding with --fully-static flag."
        else
            echo ""
            echo "✅ No MSYS2/MinGW dependencies found - good for standalone!"
        fi
        
    elif [[ "$PLATFORM" == "linux" ]]; then
        # For Linux, check for reasonable system dependencies
        DEPS=$($DEPENDENCY_CMD "$EXE_PATH" 2>/dev/null)
        echo "$DEPS"
        
        # Count non-system dependencies
        NON_SYSTEM_DEPS=$(echo "$DEPS" | grep -v -E "(linux-vdso|ld-linux|libc\.so|libdl\.so|libpthread\.so|libm\.so|librt\.so)" | wc -l)
        
        if [[ $NON_SYSTEM_DEPS -gt 1 ]]; then
            echo ""
            echo "⚠️  Warning: Non-standard dependencies found!"
            echo "This executable may require additional libraries on other systems."
        else
            echo ""
            echo "✅ Only standard system dependencies found - good for standalone!"
        fi
        
    elif [[ "$PLATFORM" == "macos" ]]; then
        DEPS=$($DEPENDENCY_CMD "$EXE_PATH" 2>/dev/null)
        echo "$DEPS"
        echo ""
        echo "ℹ️  macOS dependency analysis (manual review recommended)"
    fi
else
    echo "⚠️  Dependency checker ($DEPENDENCY_CMD) not available"
fi

echo ""
echo "🧪 Testing functionality..."

# Test basic functionality
if "$EXE_PATH" --version &>/dev/null; then
    echo "✅ --version works"
else
    echo "❌ --version failed"
fi

if "$EXE_PATH" --help &>/dev/null; then
    echo "✅ --help works"
else
    echo "❌ --help failed"
fi

echo ""
echo "📋 Summary:"
echo "----------------------------------------"
echo "Executable: $EXE_PATH"
echo "Size: $FILE_SIZE"
echo "Platform: $PLATFORM"

# Final assessment
STANDALONE_SCORE=0

if [[ "$PLATFORM" == "windows" ]]; then
    if ! $DEPENDENCY_CMD "$EXE_PATH" 2>/dev/null | grep -qi "msys\|mingw"; then
        STANDALONE_SCORE=$((STANDALONE_SCORE + 1))
    fi
elif [[ "$PLATFORM" == "linux" ]]; then
    NON_SYSTEM_DEPS=$(ldd "$EXE_PATH" 2>/dev/null | grep -v -E "(linux-vdso|ld-linux|libc\.so|libdl\.so|libpthread\.so|libm\.so|librt\.so)" | wc -l)
    if [[ $NON_SYSTEM_DEPS -le 1 ]]; then
        STANDALONE_SCORE=$((STANDALONE_SCORE + 1))
    fi
else
    STANDALONE_SCORE=$((STANDALONE_SCORE + 1))  # Assume OK for other platforms
fi

if "$EXE_PATH" --version &>/dev/null; then
    STANDALONE_SCORE=$((STANDALONE_SCORE + 1))
fi

if [[ $STANDALONE_SCORE -eq 2 ]]; then
    echo ""
    echo "🎉 EXCELLENT! This appears to be a truly standalone executable."
    echo "   You can distribute this binary to other $PLATFORM systems."
    echo ""
    echo "✅ Ready for distribution!"
elif [[ $STANDALONE_SCORE -eq 1 ]]; then
    echo ""
    echo "⚠️  PARTIALLY STANDALONE. May work on most systems but could have issues."
    echo "   Consider rebuilding with --fully-static for better compatibility."
else
    echo ""
    echo "❌ NOT STANDALONE. This executable has dependencies that may not be available"
    echo "   on other systems. Rebuild with static linking options."
fi

echo ""
echo "========================================================"

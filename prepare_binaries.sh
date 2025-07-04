#!/bin/bash

# Script to prepare compiled binaries for distribution
# This creates a compiled_binaries directory structure and copies built executables

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

echo "=================================================================="
echo "          AMCheck C++ Compiled Binaries Preparation"
echo "=================================================================="
echo

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    EXECUTABLE_NAME="amcheck"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    EXECUTABLE_NAME="amcheck"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="windows"
    EXECUTABLE_NAME="amcheck.exe"
else
    PLATFORM="unknown"
    EXECUTABLE_NAME="amcheck"
fi

print_status "Platform detected: $PLATFORM"

# Create directory structure
print_status "Creating compiled_binaries directory structure..."
mkdir -p compiled_binaries/$PLATFORM
mkdir -p compiled_binaries/$(uname -m)

if [[ "$PLATFORM" == "linux" ]]; then
    mkdir -p compiled_binaries/linux
    mkdir -p compiled_binaries/x86_64
    mkdir -p compiled_binaries/aarch64
elif [[ "$PLATFORM" == "windows" ]]; then
    mkdir -p compiled_binaries/windows
    mkdir -p compiled_binaries/win64
    mkdir -p compiled_binaries/win32
elif [[ "$PLATFORM" == "macos" ]]; then
    mkdir -p compiled_binaries/macos
    mkdir -p compiled_binaries/darwin
fi

# Find executable
EXECUTABLE=""
if [[ -f "build/bin/$EXECUTABLE_NAME" ]]; then
    EXECUTABLE="build/bin/$EXECUTABLE_NAME"
elif [[ -f "bin/$EXECUTABLE_NAME" ]]; then
    EXECUTABLE="bin/$EXECUTABLE_NAME"
elif [[ -f "$EXECUTABLE_NAME" ]]; then
    EXECUTABLE="$EXECUTABLE_NAME"
else
    print_error "Executable $EXECUTABLE_NAME not found!"
    print_error "Please build the project first"
    exit 1
fi

print_success "Found executable: $EXECUTABLE"

# Get file info
FILE_SIZE=$(du -h "$EXECUTABLE" | cut -f1)
print_status "File size: $FILE_SIZE"

# Copy to appropriate directories
print_status "Copying executable to compiled_binaries directories..."

# Copy to platform-specific directory
cp "$EXECUTABLE" "compiled_binaries/$PLATFORM/$EXECUTABLE_NAME"
print_success "Copied to compiled_binaries/$PLATFORM/"

# Copy to architecture-specific directory
cp "$EXECUTABLE" "compiled_binaries/$(uname -m)/$EXECUTABLE_NAME"
print_success "Copied to compiled_binaries/$(uname -m)/"

# Copy to root of compiled_binaries for convenience
cp "$EXECUTABLE" "compiled_binaries/$EXECUTABLE_NAME"
print_success "Copied to compiled_binaries/"

# Platform-specific additional copies
if [[ "$PLATFORM" == "linux" ]]; then
    cp "$EXECUTABLE" "compiled_binaries/linux/$EXECUTABLE_NAME"
    if [[ "$(uname -m)" == "x86_64" ]]; then
        cp "$EXECUTABLE" "compiled_binaries/x86_64/$EXECUTABLE_NAME"
    fi
elif [[ "$PLATFORM" == "windows" ]]; then
    cp "$EXECUTABLE" "compiled_binaries/windows/$EXECUTABLE_NAME"
    cp "$EXECUTABLE" "compiled_binaries/win64/$EXECUTABLE_NAME"
elif [[ "$PLATFORM" == "macos" ]]; then
    cp "$EXECUTABLE" "compiled_binaries/macos/$EXECUTABLE_NAME"
    cp "$EXECUTABLE" "compiled_binaries/darwin/$EXECUTABLE_NAME"
fi

# Create info file
print_status "Creating binary information file..."
cat > compiled_binaries/README.txt << EOF
AMCheck C++ Compiled Binaries
============================

Build Information:
- Platform: $PLATFORM
- Architecture: $(uname -m)
- Build Date: $(date)
- File Size: $FILE_SIZE
- Compiler: $(gcc --version 2>/dev/null | head -n1 || echo "Unknown")

Directory Structure:
- compiled_binaries/$EXECUTABLE_NAME           # Generic binary
- compiled_binaries/$PLATFORM/$EXECUTABLE_NAME # Platform-specific
- compiled_binaries/$(uname -m)/$EXECUTABLE_NAME        # Architecture-specific

Installation:
Linux/macOS: ./install.sh
Windows:     install_windows.bat

The installation scripts will automatically detect and use these
pre-compiled binaries if no built executable is found.

Usage:
$EXECUTABLE_NAME --help
$EXECUTABLE_NAME POSCAR                    # Basic analysis
$EXECUTABLE_NAME -b BAND.dat               # Band analysis
$EXECUTABLE_NAME -a POSCAR                 # Comprehensive search
$EXECUTABLE_NAME --ahc POSCAR              # Anomalous Hall analysis
EOF

# List created files
print_status "Created compiled binaries:"
find compiled_binaries -name "$EXECUTABLE_NAME" -type f -exec ls -lh {} \;

echo
print_success "Compiled binaries prepared successfully!"
print_status "Directory: compiled_binaries/"
print_status "Installation scripts will now automatically detect these binaries"

echo
echo "=================================================================="
echo "             Ready for Distribution!"
echo "=================================================================="

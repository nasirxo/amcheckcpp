#!/bin/bash

# Setup script for AMCheck C++ development in MSYS2
# This script checks and installs all necessary dependencies

echo "=== AMCheck C++ Setup for MSYS2 ==="
echo

# Check if running in MSYS2
if [[ -z "$MINGW_PREFIX" ]]; then
    echo "❌ Error: This script must be run from MSYS2 MinGW64 terminal"
    echo "   Please open MSYS2 MinGW64 and run this script again"
    exit 1
fi

echo "✅ Running in MSYS2 environment: $MINGW_PREFIX"
echo

# Function to check if package is installed
check_package() {
    if pacman -Qi "$1" &> /dev/null; then
        echo "✅ $1 is already installed"
        return 0
    else
        echo "❌ $1 is not installed"
        return 1
    fi
}

# Function to install package
install_package() {
    echo "📦 Installing $1..."
    if pacman -S --noconfirm "$1"; then
        echo "✅ Successfully installed $1"
    else
        echo "❌ Failed to install $1"
        exit 1
    fi
}

echo "Checking required packages..."
echo

# List of required packages
packages=(
    "mingw-w64-x86_64-cmake"
    "mingw-w64-x86_64-make" 
    "mingw-w64-x86_64-gcc"
    "mingw-w64-x86_64-eigen3"
    "mingw-w64-x86_64-spglib"
    "mingw-w64-x86_64-pkg-config"
)

# Check each package
missing_packages=()
for package in "${packages[@]}"; do
    if ! check_package "$package"; then
        missing_packages+=("$package")
    fi
done

# Install missing packages
if [ ${#missing_packages[@]} -gt 0 ]; then
    echo
    echo "Installing missing packages..."
    for package in "${missing_packages[@]}"; do
        install_package "$package"
    done
else
    echo
    echo "✅ All required packages are already installed!"
fi

echo
echo "Checking tool versions..."
echo "CMake: $(cmake --version | head -1)"
echo "GCC: $(gcc --version | head -1)"
echo "Make: $(make --version | head -1)"

# Check Eigen3
eigen_path="$MINGW_PREFIX/include/eigen3"
if [ -d "$eigen_path" ]; then
    echo "✅ Eigen3 found at: $eigen_path"
else
    echo "⚠️  Eigen3 path not found at expected location"
fi

# Check spglib
spglib_lib="$MINGW_PREFIX/lib/libsymspg.a"
spglib_header="$MINGW_PREFIX/include/spglib.h"
if [ -f "$spglib_lib" ] && [ -f "$spglib_header" ]; then
    echo "✅ spglib found at: $MINGW_PREFIX/lib and $MINGW_PREFIX/include"
else
    echo "⚠️  spglib not found at expected locations"
    echo "   Looking for: $spglib_lib and $spglib_header"
fi

echo
echo "=== Setup Complete ==="
echo
echo "Next steps:"
echo "1. Build the project:"
echo "   ./build_msys2.sh"
echo
echo "2. Or use VS Code tasks:"
echo "   - Ctrl+Shift+P → 'Tasks: Run Task' → 'Build AMCheck C++ (MSYS2)'"
echo
echo "3. Run the program:"
echo "   ./build/bin/amcheck.exe --help"

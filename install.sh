#!/bin/bash

# Universal AMCheck C++ Installer
# Works on Linux, macOS, and WSL

set -e

# Detect platform
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="windows"
else
    PLATFORM="unknown"
fi

echo "=================================================================="
echo "              AMCheck C++ Universal Installer"
echo "=================================================================="
echo "Platform detected: $PLATFORM"
echo

case $PLATFORM in
    "linux"|"macos")
        if [[ -f "install_linux.sh" ]]; then
            echo "Running Linux/macOS installer..."
            chmod +x install_linux.sh
            ./install_linux.sh "$@"
        else
            echo "Error: install_linux.sh not found"
            exit 1
        fi
        ;;
    "windows")
        if [[ -f "install_windows.bat" ]]; then
            echo "Running Windows installer..."
            cmd.exe /c install_windows.bat "$@"
        else
            echo "Error: install_windows.bat not found"
            exit 1
        fi
        ;;
    *)
        echo "Unsupported platform: $OSTYPE"
        echo "Please run the appropriate installer manually:"
        echo "  Linux/macOS: ./install_linux.sh"
        echo "  Windows:     install_windows.bat"
        exit 1
        ;;
esac

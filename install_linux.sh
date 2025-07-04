#!/bin/bash

# AMCheck C++ Installation Script for Linux
# This script copies the executable to /usr/local/bin for global access

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
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

# Check if running as root for system installation
check_permissions() {
    if [[ $EUID -eq 0 ]]; then
        INSTALL_DIR="/usr/local/bin"
        INSTALL_TYPE="system-wide"
        USE_SUDO=""
    else
        # Check if user has sudo access
        if sudo -n true 2>/dev/null; then
            INSTALL_DIR="/usr/local/bin"
            INSTALL_TYPE="system-wide (with sudo)"
            USE_SUDO="sudo"
        else
            # Fall back to user's local bin
            INSTALL_DIR="$HOME/.local/bin"
            INSTALL_TYPE="user-local"
            USE_SUDO=""
            mkdir -p "$INSTALL_DIR"
        fi
    fi
}

# Find the executable
find_executable() {
    if [[ -f "build/bin/amcheck" ]]; then
        EXECUTABLE="build/bin/amcheck"
    elif [[ -f "bin/amcheck" ]]; then
        EXECUTABLE="bin/amcheck"
    elif [[ -f "amcheck" ]]; then
        EXECUTABLE="amcheck"
    elif [[ -f "compiled_binaries/amcheck" ]]; then
        EXECUTABLE="compiled_binaries/amcheck"
        print_status "Using pre-compiled binary from compiled_binaries/"
    elif [[ -f "compiled_binaries/linux/amcheck" ]]; then
        EXECUTABLE="compiled_binaries/linux/amcheck"
        print_status "Using pre-compiled Linux binary from compiled_binaries/linux/"
    elif [[ -f "compiled_binaries/$(uname -m)/amcheck" ]]; then
        EXECUTABLE="compiled_binaries/$(uname -m)/amcheck"
        print_status "Using pre-compiled binary for $(uname -m) from compiled_binaries/$(uname -m)/"
    else
        print_error "AMCheck executable not found!"
        print_error "Please build the project first using: ./build.sh"
        print_error "Expected locations: build/bin/amcheck, bin/amcheck, amcheck,"
        print_error "                   or compiled_binaries/[linux|$(uname -m)]/amcheck"
        exit 1
    fi
}

# Check if system binary directory has AMCheck and offer to use compiled binaries
check_system_binaries() {
    # Check if amcheck already exists in system paths
    if command -v amcheck &> /dev/null; then
        EXISTING_PATH=$(which amcheck)
        print_warning "AMCheck already installed at: $EXISTING_PATH"
        
        # Check version if possible
        if EXISTING_VERSION=$(amcheck --version 2>/dev/null | head -n1); then
            print_status "Existing version: $EXISTING_VERSION"
        fi
        
        echo
        read -p "Do you want to replace the existing installation? [y/N]: " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_status "Keeping existing installation"
            exit 0
        fi
    fi
    
    # If /usr/bin or /usr/local/bin is empty of amcheck, suggest using compiled binaries
    if [[ ! -f "/usr/bin/amcheck" && ! -f "/usr/local/bin/amcheck" ]]; then
        if [[ -d "compiled_binaries" ]]; then
            print_status "System binary directories are clean"
            print_status "Found compiled_binaries directory - checking for pre-built binaries"
            
            # List available binaries
            if ls compiled_binaries/*/amcheck 2>/dev/null || ls compiled_binaries/amcheck 2>/dev/null; then
                print_status "Available pre-compiled binaries:"
                ls -la compiled_binaries/*/amcheck 2>/dev/null || ls -la compiled_binaries/amcheck 2>/dev/null
            fi
        fi
    fi
}

# Install function
install_amcheck() {
    print_status "Installing AMCheck C++ to $INSTALL_DIR..."
    
    $USE_SUDO cp "$EXECUTABLE" "$INSTALL_DIR/amcheck"
    $USE_SUDO chmod +x "$INSTALL_DIR/amcheck"
    
    print_success "Executable copied to $INSTALL_DIR/amcheck"
}

# Update PATH if needed
update_path() {
    if [[ "$INSTALL_DIR" == "$HOME/.local/bin" ]]; then
        # Check if ~/.local/bin is in PATH
        if [[ ":$PATH:" != *":$HOME/.local/bin:"* ]]; then
            print_warning "~/.local/bin is not in your PATH"
            print_status "Adding ~/.local/bin to your PATH..."
            
            # Detect shell and update appropriate config file
            if [[ "$SHELL" == *"zsh"* ]]; then
                echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
                print_success "Added to ~/.zshrc"
            elif [[ "$SHELL" == *"bash"* ]]; then
                echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
                print_success "Added to ~/.bashrc"
            else
                print_warning "Please add 'export PATH=\"\$HOME/.local/bin:\$PATH\"' to your shell config file"
            fi
            
            print_warning "Please restart your terminal or run: source ~/.bashrc (or ~/.zshrc)"
        fi
    fi
}

# Verify installation
verify_installation() {
    print_status "Verifying installation..."
    
    # Add current install dir to PATH for this verification
    export PATH="$INSTALL_DIR:$PATH"
    
    if command -v amcheck &> /dev/null; then
        print_success "AMCheck is now globally accessible!"
        print_status "Testing executable..."
        
        if amcheck --version &> /dev/null; then
            print_success "Installation successful!"
            echo
            echo "=================================================================="
            echo "                     Usage Examples:"
            echo "=================================================================="
            echo "  amcheck --help                    # Show help"
            echo "  amcheck --version                 # Show version"
            echo "  amcheck POSCAR                    # Analyze crystal structure"
            echo "  amcheck -b BAND.dat               # Analyze band structure"
            echo "  amcheck -a POSCAR                 # Comprehensive search"
            echo "  amcheck --ahc POSCAR              # Anomalous Hall analysis"
            echo "=================================================================="
        else
            print_warning "Executable installed but may have dependency issues"
            print_status "Try running: amcheck --help"
        fi
    else
        print_error "Installation failed - amcheck not found in PATH"
        print_error "You may need to restart your terminal or update PATH manually"
        exit 1
    fi
}

# Uninstall function
uninstall_amcheck() {
    print_status "Uninstalling AMCheck C++..."
    
    if [[ -f "$INSTALL_DIR/amcheck" ]]; then
        $USE_SUDO rm -f "$INSTALL_DIR/amcheck"
        print_success "AMCheck removed from $INSTALL_DIR"
    else
        print_warning "AMCheck not found in $INSTALL_DIR"
    fi
}

# Main execution
main() {
    echo "=================================================================="
    echo "                 AMCheck C++ Installation Script"
    echo "=================================================================="
    echo
    
    # Check for uninstall option
    if [[ "$1" == "--uninstall" || "$1" == "-u" ]]; then
        check_permissions
        uninstall_amcheck
        exit 0
    fi
    
    check_system_binaries
    find_executable
    print_success "Found executable: $EXECUTABLE"
    
    # Check file size and dependencies
    FILE_SIZE=$(du -h "$EXECUTABLE" | cut -f1)
    print_status "Executable size: $FILE_SIZE"
    
    check_permissions
    print_status "Installation type: $INSTALL_TYPE"
    print_status "Installation directory: $INSTALL_DIR"
    
    # Ask for confirmation
    echo
    read -p "Proceed with installation? [Y/n]: " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Nn]$ ]]; then
        print_status "Installation cancelled"
        exit 0
    fi
    
    install_amcheck
    update_path
    verify_installation
    
    echo
    print_success "AMCheck C++ installation complete!"
    echo "=================================================================="
}

# Show help
show_help() {
    echo "AMCheck C++ Installation Script for Linux"
    echo
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "OPTIONS:"
    echo "  --help, -h        Show this help message"
    echo "  --uninstall, -u   Uninstall AMCheck from system"
    echo
    echo "Installation Locations:"
    echo "  System-wide:      /usr/local/bin (requires sudo)"
    echo "  User-local:       ~/.local/bin (fallback if no sudo)"
    echo
    echo "Examples:"
    echo "  $0                # Install AMCheck"
    echo "  $0 --uninstall    # Remove AMCheck"
}

# Check arguments
if [[ "$1" == "--help" || "$1" == "-h" ]]; then
    show_help
    exit 0
fi

# Check if script is being sourced or executed
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi

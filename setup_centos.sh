#!/bin/bash

# AMCheck C++ - CentOS/RHEL Setup Script
# This script installs all required dependencies for AMCheck C++ on CentOS/RHEL systems

set -e

echo "=========================================="
echo "AMCheck C++ - CentOS/RHEL Setup Script"
echo "=========================================="

# Detect CentOS/RHEL version
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS_NAME=$NAME
    OS_VERSION=$VERSION_ID
    echo "Detected: $OS_NAME $OS_VERSION"
else
    echo "Cannot detect OS version. Assuming CentOS/RHEL 7"
    OS_VERSION="7"
fi

# Determine package manager and commands
if command -v dnf &> /dev/null; then
    PKG_MGR="dnf"
    INSTALL_CMD="sudo dnf install -y"
    GROUP_INSTALL_CMD="sudo dnf groupinstall -y"
    REPO_CMD="sudo dnf config-manager --set-enabled"
elif command -v yum &> /dev/null; then
    PKG_MGR="yum"
    INSTALL_CMD="sudo yum install -y"
    GROUP_INSTALL_CMD="sudo yum groupinstall -y"
    REPO_CMD="sudo yum config-manager --set-enabled"
else
    echo "Error: Neither dnf nor yum found. Cannot proceed."
    exit 1
fi

echo "Using package manager: $PKG_MGR"

# Function to check if package is installed
is_installed() {
    if [ "$PKG_MGR" = "dnf" ]; then
        dnf list installed "$1" &> /dev/null
    else
        yum list installed "$1" &> /dev/null
    fi
}

# Install EPEL repository
echo "----------------------------------------"
echo "Installing EPEL repository..."
echo "----------------------------------------"
if ! is_installed epel-release; then
    $INSTALL_CMD epel-release
    echo "✅ EPEL repository installed"
else
    echo "✅ EPEL repository already installed"
fi

# Enable PowerTools/CRB for development packages
echo "----------------------------------------"
echo "Enabling development repositories..."
echo "----------------------------------------"
if [[ "$OS_VERSION" == "8" ]]; then
    $REPO_CMD powertools || echo "PowerTools repository might already be enabled"
elif [[ "$OS_VERSION" == "9" ]] || [[ "$OS_NAME" == *"Stream"* ]]; then
    $REPO_CMD crb || echo "CRB repository might already be enabled"
elif [[ "$OS_VERSION" == "7" ]]; then
    echo "CentOS 7 detected - no additional repositories needed"
fi

# Install development tools
echo "----------------------------------------"
echo "Installing development tools..."
echo "----------------------------------------"
$GROUP_INSTALL_CMD "Development Tools"

# Install essential packages
echo "----------------------------------------"
echo "Installing essential packages..."
echo "----------------------------------------"
ESSENTIAL_PACKAGES="gcc gcc-c++ make git wget"

if [[ "$OS_VERSION" == "7" ]]; then
    ESSENTIAL_PACKAGES="$ESSENTIAL_PACKAGES cmake3"
else
    ESSENTIAL_PACKAGES="$ESSENTIAL_PACKAGES cmake"
fi

$INSTALL_CMD $ESSENTIAL_PACKAGES

# Create cmake symlink for CentOS 7
if [[ "$OS_VERSION" == "7" ]] && [ ! -L /usr/local/bin/cmake ]; then
    echo "Creating cmake symlink for CentOS 7..."
    sudo mkdir -p /usr/local/bin
    sudo ln -sf /usr/bin/cmake3 /usr/local/bin/cmake
    echo "✅ CMake3 symlink created"
fi

# Install Eigen3
echo "----------------------------------------"
echo "Installing Eigen3..."
echo "----------------------------------------"
if [[ "$OS_VERSION" == "7" ]]; then
    echo "Installing Eigen3 from source for CentOS 7..."
    cd /tmp
    if [ ! -d "eigen-3.4.0" ]; then
        wget -q https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
        tar -xzf eigen-3.4.0.tar.gz
    fi
    cd eigen-3.4.0
    mkdir -p build && cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
    make -j$(nproc)
    sudo make install
    echo "✅ Eigen3 installed from source"
else
    if ! is_installed eigen3-devel; then
        $INSTALL_CMD eigen3-devel
        echo "✅ Eigen3 installed from package"
    else
        echo "✅ Eigen3 already installed"
    fi
fi

# Install spglib from source
echo "----------------------------------------"
echo "Installing spglib from source..."
echo "----------------------------------------"
cd /tmp
if [ ! -d "spglib" ]; then
    git clone https://github.com/spglib/spglib.git
fi
cd spglib
git pull  # Update to latest
mkdir -p build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
sudo ldconfig
echo "✅ spglib installed from source"

# Install CUDA (optional)
echo "----------------------------------------"
echo "CUDA Installation (optional)"
echo "----------------------------------------"
echo "Do you want to install NVIDIA CUDA Toolkit? (y/N): "
read -r INSTALL_CUDA

if [[ "$INSTALL_CUDA" =~ ^[Yy]$ ]]; then
    echo "Installing CUDA Toolkit..."
    
    if [[ "$OS_VERSION" == "7" ]]; then
        # CentOS 7
        CUDA_REPO_URL="https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/cuda-repo-rhel7-12.3.2-1.x86_64.rpm"
        sudo yum install -y $CUDA_REPO_URL
        sudo yum clean all
        sudo yum install -y cuda-toolkit-12-3
    else
        # CentOS 8/9
        sudo dnf config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
        sudo dnf clean all
        sudo dnf install -y cuda-toolkit
    fi
    
    # Add CUDA to PATH
    echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
    echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
    
    echo "✅ CUDA Toolkit installed"
    echo "⚠️  Please restart your terminal or run: source ~/.bashrc"
else
    echo "Skipping CUDA installation"
fi

# Verify installations
echo "----------------------------------------"
echo "Verifying installations..."
echo "----------------------------------------"

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    echo "✅ CMake: $CMAKE_VERSION"
else
    echo "❌ CMake not found"
fi

# Check GCC
if command -v gcc &> /dev/null; then
    GCC_VERSION=$(gcc --version | head -n1 | awk '{print $4}')
    echo "✅ GCC: $GCC_VERSION"
else
    echo "❌ GCC not found"
fi

# Check Eigen3
if find /usr -name "Eigen" -type d 2>/dev/null | grep -q .; then
    EIGEN_PATH=$(find /usr -name "Eigen" -type d 2>/dev/null | head -1)
    echo "✅ Eigen3: Found at $EIGEN_PATH"
else
    echo "❌ Eigen3 not found"
fi

# Check spglib
if ldconfig -p 2>/dev/null | grep -q "symspg"; then
    echo "✅ spglib: Found in system libraries"
else
    echo "❌ spglib not found in system libraries"
fi

# Check CUDA (if requested)
if [[ "$INSTALL_CUDA" =~ ^[Yy]$ ]]; then
    if command -v nvcc &> /dev/null; then
        NVCC_VERSION=$(nvcc --version | grep "release" | awk '{print $6}')
        echo "✅ CUDA: $NVCC_VERSION"
    else
        echo "❌ CUDA not found (may need to restart terminal)"
    fi
fi

echo "----------------------------------------"
echo "Setup completed!"
echo "----------------------------------------"
echo "You can now build AMCheck C++ with:"
echo "  cd /path/to/amcheckcpp/cpp"
echo "  ./build.sh --no-cuda    # CPU-only build"
echo "  ./build.sh              # With CUDA (if installed)"
echo ""
echo "For detailed instructions, see: INSTALL_CENTOS.md"

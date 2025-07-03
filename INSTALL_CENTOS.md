# AMCheck C++ Installation Guide for CentOS/RHEL

This guide covers installing AMCheck C++ and its dependencies on CentOS/RHEL systems.

## 🚨 URGENT FIX FOR YOUR CURRENT ERROR

**You're getting "CMake 3.5 or higher is required" but have version 2.8.12.2**

**Quick Fix:**
```bash
# Install cmake3 (available on CentOS 7)
sudo yum install -y cmake3

# Use cmake3 directly instead of cmake
cd /path/to/amcheckcpp
mkdir -p build && cd build
cmake3 .. -DCMAKE_BUILD_TYPE=Release -DENABLE_CUDA=OFF
make -j$(nproc)
```

**OR create a symlink:**
```bash
sudo yum install -y cmake3
sudo ln -sf /usr/bin/cmake3 /usr/local/bin/cmake
export PATH=/usr/local/bin:$PATH

# Now retry your build
cd /path/to/amcheckcpp
./build.sh --no-cuda
```

## Prerequisites

### 1. Enable Required Repositories

For CentOS 7:
```bash
# Enable EPEL repository
sudo yum install epel-release

# Enable PowerTools (for development packages)
sudo yum config-manager --set-enabled PowerTools
```

For CentOS Stream 8/9 or RHEL 8/9:
```bash
# Enable EPEL repository
sudo dnf install epel-release

# Enable PowerTools/CRB repository
sudo dnf config-manager --set-enabled powertools  # CentOS 8
sudo dnf config-manager --set-enabled crb         # CentOS Stream 9
```

For Rocky Linux/AlmaLinux:
```bash
sudo dnf install epel-release
sudo dnf config-manager --set-enabled powertools
```

### 2. Install Development Tools

For CentOS 7:
```bash
sudo yum groupinstall "Development Tools"
sudo yum install gcc gcc-c++ make cmake3
```

For CentOS Stream 8/9, Rocky/Alma:
```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install gcc gcc-c++ make cmake
```

## Required Dependencies

### 1. Install CMake (Modern Version)

**For CentOS 7 (needs newer CMake):**
```bash
# Remove old cmake if installed
sudo yum remove cmake

# Install cmake3 from EPEL
sudo yum install cmake3
sudo ln -sf /usr/bin/cmake3 /usr/local/bin/cmake
```

**For CentOS Stream 8/9:**
```bash
sudo dnf install cmake
```

Verify CMake version (requires >= 3.15):
```bash
cmake --version
```

### 2. Install Eigen3

**Method 1: From package manager (CentOS 8/9):**
```bash
sudo dnf install eigen3-devel
```

**Method 2: From source (CentOS 7 or if package not available):**
```bash
cd /tmp
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar -xzf eigen-3.4.0.tar.gz
cd eigen-3.4.0
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make -j$(nproc)
sudo make install
```

### 3. Install spglib (Space Group Library)

**Method 1: From source (recommended):**
```bash
cd /tmp
git clone https://github.com/spglib/spglib.git
cd spglib
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install

# Update library cache
sudo ldconfig
```

**Method 2: Using conda (if available):**
```bash
conda install -c conda-forge spglib
```

### 4. Install Threading Support
```bash
# Already included with gcc, but ensure pthread is available
sudo dnf install glibc-devel  # or sudo yum install glibc-devel for CentOS 7
```

## Optional: CUDA Support

### Install NVIDIA CUDA Toolkit

**For CentOS 7:**
```bash
# Add NVIDIA repository
sudo yum install -y https://developer.download.nvidia.com/compute/cuda/repos/rhel7/x86_64/cuda-repo-rhel7-12.3.2-1.x86_64.rpm
sudo yum clean all
sudo yum install cuda-toolkit-12-3
```

**For CentOS Stream 8/9:**
```bash
# Add NVIDIA repository
sudo dnf config-manager --add-repo https://developer.download.nvidia.com/compute/cuda/repos/rhel8/x86_64/cuda-rhel8.repo
sudo dnf clean all
sudo dnf install cuda-toolkit
```

**Post-installation:**
```bash
# Add CUDA to PATH
echo 'export PATH=/usr/local/cuda/bin:$PATH' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc

# Verify installation
nvcc --version
```

## Building AMCheck C++

### 1. Clone the Repository
```bash
git clone https://github.com/nasirxo/amcheckcpp.git
cd amcheckcpp/cpp
```

### 2. Build Options

**Basic build (CPU only):**
```bash
./build.sh --no-cuda
```

**With CUDA support:**
```bash
./build.sh
```

**Fully static build:**
```bash
./build.sh --fully-static --no-cuda
```

### 3. Manual CMake Build (if build script fails)

```bash
mkdir build && cd build

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC=ON \
    -DENABLE_CUDA=OFF

# Build
make -j$(nproc)

# Test
./bin/amcheck --version
```

## Troubleshooting

### Common Issues

**1. CMake version too old:**
```bash
# For CentOS 7, use cmake3
cmake3 --version
# If still too old, install from source:
cd /tmp
wget https://github.com/Kitware/CMake/releases/download/v3.27.7/cmake-3.27.7.tar.gz
tar -xzf cmake-3.27.7.tar.gz
cd cmake-3.27.7
./bootstrap --prefix=/usr/local
make -j$(nproc)
sudo make install
```

**2. Eigen3 not found:**
```bash
# Check if installed
find /usr -name "Eigen" -type d 2>/dev/null
# If not found, install from source as shown above
```

**3. spglib not found:**
```bash
# Check if library is available
ldconfig -p | grep symspg
# If not found, install from source as shown above
```

**4. Library path issues:**
```bash
# Add library paths
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:$LD_LIBRARY_PATH
sudo ldconfig
```

**5. Permissions issues:**
```bash
# Ensure proper permissions for installation directories
sudo chown -R $(whoami):$(whoami) /usr/local/include
sudo chown -R $(whoami):$(whoami) /usr/local/lib
```

## Verification

After installation, verify everything works:

```bash
# Check dependencies
ldd ./bin/amcheck

# Test basic functionality
./bin/amcheck --version
./bin/amcheck --help

# Test with a sample structure (if available)
./bin/amcheck ../examples/sample_POSCAR
```

## Package Installation Summary

**CentOS 7 (one-liner):**
```bash
sudo yum install epel-release && \
sudo yum config-manager --set-enabled PowerTools && \
sudo yum groupinstall "Development Tools" && \
sudo yum install gcc gcc-c++ make cmake3 git wget
```

**CentOS Stream 8/9 (one-liner):**
```bash
sudo dnf install epel-release && \
sudo dnf config-manager --set-enabled powertools && \
sudo dnf groupinstall "Development Tools" && \
sudo dnf install gcc gcc-c++ make cmake git wget eigen3-devel
```

## Performance Tips

1. **Use multiple cores for compilation:**
   ```bash
   make -j$(nproc)
   ```

2. **Build with optimizations:**
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

3. **Enable CUDA for large structures:**
   ```bash
   ./build.sh  # CUDA enabled by default
   ```

4. **Use static linking for deployment:**
   ```bash
   ./build.sh --fully-static
   ```

## Support

If you encounter issues:

1. Check the build log for specific error messages
2. Ensure all dependencies are properly installed
3. Try building without CUDA first: `./build.sh --no-cuda`
4. Use manual CMake build for more control
5. Check the GitHub issues page for known problems

For additional help:
- Email: nasiraliphy@gmail.com
- GitHub: https://github.com/nasirxo/amcheckcpp

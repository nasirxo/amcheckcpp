# AMCheck C++ - Altermagnet Detection Tool

A high-performance C++ implementation of the altermagnet checker tool for analyzing crystal structures and detecting altermagnetic properties. This tool provides comprehensive crystallographic analysis using advanced symmetry operations and multithreaded computation.

![License](https://img.shields.io/badge/license-BSD--3--Clause-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Windows%20%7C%20macOS-green.svg)

## Authors & Credits

### Development Team
- **Nasir Ali** - Lead Developer  
  📧 Contact: [nasiraliphy@gmail.com](mailto:nasiraliphy@gmail.com)
  
- **Shah Faisal** - Co-Developer  
  📧 Contact: [shahf8885@gmail.com](mailto:shahf8885@gmail.com)

### Academic Affiliation
**Department of Physics**  
**Quaid-i-Azam University, Islamabad, Pakistan**

### Supervision
**Prof. Dr. Gul Rahman** - Academic Supervisor  
📧 Contact: [gulrahman@qau.edu.pk](mailto:gulrahman@qau.edu.pk)

### Repository
🔗 **GitHub**: [https://github.com/nasirxo/amcheckcpp](https://github.com/nasirxo/amcheckcpp)

---

## Overview

AMCheck C++ is a sophisticated crystallographic analysis tool designed to:

- **Detect Altermagnetic Materials**: Identify materials exhibiting altermagnetic properties through symmetry analysis
- **Multithreaded Computation**: Utilize all available CPU cores for comprehensive spin configuration searches
- **Anomalous Hall Effect Analysis**: Calculate and analyze anomalous Hall coefficients
- **Cross-Platform Support**: Run seamlessly on Linux, Windows (MSYS2), and macOS
- **High Performance**: Optimized C++ implementation with Eigen3 and spglib integration
- **Standalone Distribution**: Self-contained executables that work without installing dependencies

### What are Altermagnets?

Altermagnets are a novel class of magnetic materials that combine properties of ferromagnets and antiferromagnets. They exhibit:
- Zero net magnetization (like antiferromagnets)
- Lifted spin degeneracy in electronic bands (like ferromagnets)
- Unique topological and transport properties

## Features

### Core Functionality
- ✅ **Crystal Structure Analysis**: VASP POSCAR file parsing and validation
- ✅ **Symmetry Operations**: Complete space group detection using spglib
- ✅ **Interactive Spin Assignment**: User-friendly orbit-based magnetic configuration
- ✅ **Altermagnet Detection**: Advanced symmetry-based analysis algorithm
- ✅ **Multithreaded Search**: Comprehensive spin configuration exploration using all CPU cores
- ✅ **File Output**: Detailed results saved to structured text files
- ✅ **Anomalous Hall Coefficient Analysis**: Magnetic transport property calculations
- ✅ **Standalone Executables**: Self-contained binaries requiring no external dependencies

### Analysis Modes
1. **Standard Mode**: Single configuration altermagnet analysis
2. **Comprehensive Search Mode** (`-a`): Multithreaded exploration of all possible spin configurations
3. **Anomalous Hall Coefficient Mode** (`--ahc`): Magnetic transport property analysis

## Dependencies & System Requirements

### System Requirements
- **CMake**: 3.15 or higher
- **Compiler**: C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **Memory**: 4GB RAM minimum (8GB+ recommended for large structures)
- **CPU**: Multi-core CPU recommended for optimal performance in search mode

### Required Libraries
- **Eigen3**: Linear algebra operations
- **spglib**: Crystal symmetry analysis and space group detection
- **Threading**: Standard library threading support (included in C++17)

### Installing Dependencies

#### Linux (Ubuntu/Debian)
```bash
# Update package list
sudo apt-get update

# Install build tools and dependencies
sudo apt-get install cmake build-essential git
sudo apt-get install libeigen3-dev libsymspg-dev

# Alternative if libsymspg-dev is not available:
sudo apt-get install libspglib-dev

# For newer Ubuntu versions, you might need:
sudo apt-get install libspglib1 libspglib-dev

# Verify installation
cmake --version
gcc --version
```

#### Linux (Fedora/RHEL/CentOS)
```bash
# Install build tools and dependencies
sudo dnf install cmake gcc-c++ git
sudo dnf install eigen3-devel spglib-devel

# Alternative for older systems
sudo yum install cmake gcc-c++ git
sudo yum install eigen3-devel spglib-devel

# Note: Package names may vary. If spglib-devel is not found, try:
sudo dnf install libsymspg-devel
# OR for newer versions:
sudo dnf install libspglib-devel
```

#### Windows (MSYS2) - Recommended Method
```bash
# 1. Download and install MSYS2 from https://www.msys2.org/
# 2. Open MSYS2 MinGW64 terminal (NOT the MSYS2 terminal)
# 3. Update MSYS2
pacman -Syu

# 4. Install development tools and dependencies
pacman -S mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-make \
          mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-eigen3 \
          git

# Install spglib (choose one based on your environment):
# For Clang environment:
pacman -S mingw-w64-clang-x86_64-spglib
# OR for UCRT environment:
pacman -S mingw-w64-ucrt-x86_64-spglib

# 5. Verify installation
cmake --version
g++ --version
```

#### macOS (Homebrew)
```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake eigen spglib git

# Verify installation
cmake --version
clang++ --version
```

#### Windows (vcpkg) - Alternative Method
```cmd
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install eigen3:x64-windows
.\vcpkg install spglib:x64-windows

# Integrate with Visual Studio
.\vcpkg integrate install
```

## Building & Compilation

⚠️ **Important**: If you're switching between Windows (MSYS2) and Linux/WSL environments, run `./clean.sh` first to avoid build conflicts.

### Standalone Binaries 🎯

**AMCheck C++ is designed to create standalone executables that work without requiring users to install dependencies.** The build system automatically enables static linking to ensure portability.

### Quick Start (Automated Build)

#### Linux/macOS - Standalone Build
```bash
# Clone the repository (if not already done)
git clone https://github.com/nasirxo/amcheckcpp.git
cd amcheckcpp/cpp

# Clean any existing build (recommended)
chmod +x clean.sh build.sh
./clean.sh

# Build standalone executable (default)
./build.sh

# Build with fully static linking (all libraries embedded)
./build.sh --fully-static

# Build with dynamic linking (requires dependencies on target system)
./build.sh --no-static

# Executable will be created at: build/bin/amcheck
```

#### Windows (MSYS2) - Standalone Build
```bash
# From MSYS2 MinGW64 terminal
git clone https://github.com/nasirxo/amcheckcpp.git
cd amcheckcpp/cpp

# Make scripts executable and clean
chmod +x clean.sh build_msys2.sh
./clean.sh

# Build standalone executable (default)
./build_msys2.sh

# Build with fully static linking (all libraries embedded)
./build_msys2.sh --fully-static

# Build with dynamic linking (requires MSYS2 on target system)
./build_msys2.sh --no-static

# Executable will be created at: build/bin/amcheck.exe
```

### Standalone Binary Features

✅ **No External Dependencies**: The executable includes all required libraries
✅ **Cross-System Compatibility**: Run on any compatible system without installation
✅ **Portable**: Single executable file that can be copied anywhere
✅ **Self-Contained**: No need for users to install Eigen3, spglib, or other dependencies

### Build Options

| Flag | Description | Use Case |
|------|-------------|----------|
| Default | Static linking of C++ runtime | **Recommended** - Portable with minimal dependencies |
| `--fully-static` | All libraries statically linked | Maximum portability, larger file size |
| `--no-static` | Dynamic linking | Smaller file, requires dependencies on target system |

### Manual Build Process

If the automated scripts don't work, you can build manually:

#### Linux/macOS Manual Build
```bash
# Create and enter build directory
mkdir build && cd build

# Configure with CMake for standalone build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON

# For fully static linking (optional)
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_FULLY_STATIC=ON

# For dynamic linking (not recommended for distribution)
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=OFF

# Build with all available cores
make -j$(nproc)

# Executable: build/bin/amcheck
```

#### Windows (MSYS2) Manual Build
```bash
# From MSYS2 MinGW64 terminal
mkdir build && cd build

# Configure with MSYS Makefiles for standalone build
cmake .. -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON

# For fully static linking (optional)
cmake .. -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_STATIC=ON -DBUILD_FULLY_STATIC=ON

# Build with all available cores
make -j$(nproc)

# Executable: build/bin/amcheck.exe
```

#### Windows (Visual Studio) Manual Build
```cmd
# From Windows Command Prompt or PowerShell
mkdir build && cd build

# Configure for Visual Studio
cmake .. -G "Visual Studio 16 2019" -A x64

# Build
cmake --build . --config Release --parallel

# Executable: build/Release/amcheck.exe
```

### Build Verification

After successful compilation, verify the build and check standalone status:

```bash
# Check if executable exists
ls -la build/bin/amcheck*

# Test basic functionality
./build/bin/amcheck --version
./build/bin/amcheck --help

# Check dependencies (Linux)
ldd build/bin/amcheck  # Should show minimal system dependencies

# Check dependencies (Windows/MSYS2)
ldd build/bin/amcheck.exe  # Should show minimal system dependencies

# Run a quick test (if you have a POSCAR file)
./build/bin/amcheck POSCAR
```

### Standalone Binary Verification

### Standalone Binary Verification

#### Automated Verification Script
We provide a comprehensive verification script to test standalone status:

```bash
# Make the verification script executable
chmod +x verify_standalone.sh

# Run the verification
./verify_standalone.sh

# Example output:
# ========================================================
#          AMCheck Standalone Binary Verification
# ========================================================
# Platform detected: linux
# ✅ Executable found: build/bin/amcheck
# 📦 File size: 2.1M
# ✅ Only standard system dependencies found - good for standalone!
# ✅ --version works
# ✅ --help works
# 🎉 EXCELLENT! This appears to be a truly standalone executable.
#    You can distribute this binary to other linux systems.
# ✅ Ready for distribution!
```

#### Manual Checking Static Linking Success

**Linux:**
```bash
# Check for dynamic dependencies
ldd build/bin/amcheck

# Expected output for standalone build:
#   linux-vdso.so.1 (0x...)
#   libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x...)
#   libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x...)
#   libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x...)
#   /lib64/ld-linux-x86-64.so.2 (0x...)

# Check file size (static builds are larger)
ls -lh build/bin/amcheck
```

**Windows (MSYS2):**
```bash
# Check for DLL dependencies
ldd build/bin/amcheck.exe

# Expected output for standalone build:
#   ntdll.dll => /c/WINDOWS/SYSTEM32/ntdll.dll (0x...)
#   KERNEL32.DLL => /c/WINDOWS/System32/KERNEL32.DLL (0x...)
#   KERNELBASE.dll => /c/WINDOWS/System32/KERNELBASE.dll (0x...)
#   msvcrt.dll => /c/WINDOWS/System32/msvcrt.dll (0x...)

# No mingw64 or msys2 DLLs should be listed for standalone build

# Check file size
ls -lh build/bin/amcheck.exe
```

### Distribution and Deployment

#### Standalone Executable Features
- **Single File**: Just copy the executable - no installation needed
- **No Dependencies**: Works on systems without development tools
- **Cross-Platform**: Linux binary works on most Linux distributions
- **Windows Portable**: .exe works on Windows 7+ without MSYS2/MinGW

#### Deployment Examples
```bash
# Linux: Copy to any Linux system
scp build/bin/amcheck user@remote-server:/usr/local/bin/

# Windows: Copy to any Windows system
# Simply copy amcheck.exe to the target system

# Create distribution package
mkdir amcheck-standalone
cp build/bin/amcheck* amcheck-standalone/
cp README.md amcheck-standalone/
tar -czf amcheck-standalone.tar.gz amcheck-standalone/
```

## Usage Guide

### Command Line Interface

```bash
amcheck [OPTIONS] <structure_file>
```

### Available Options

| Option | Long Form | Description |
|--------|-----------|-------------|
| `-h` | `--help` | Show detailed help message |
| `-v` | `--verbose` | Enable detailed output with debug information |
| `--version` | | Display version and author information |
| `-s <value>` | `--symprec <value>` | Set symmetry precision (default: 1e-3) |
| `-t <value>` | `--tolerance <value>` | Set numerical tolerance (default: 1e-3) |
| `-a` | `--search-all` | **Multithreaded comprehensive spin search** |
| `--ahc` | | Anomalous Hall Coefficient analysis mode |

### Usage Examples

#### 1. Basic Altermagnet Analysis
```bash
# Simple analysis with interactive spin assignment
./build/bin/amcheck POSCAR

# Verbose output with detailed symmetry information
./build/bin/amcheck -v POSCAR

# Custom tolerance and symmetry precision
./build/bin/amcheck -s 1e-5 -t 1e-5 POSCAR
```

#### 2. Comprehensive Multithreaded Search ⭐ NEW!
```bash
# Search ALL possible spin configurations using all CPU cores
./build/bin/amcheck -a POSCAR

# Example output file: POSCAR_amcheck_results_20250103_143025.txt
# Contains detailed results for all altermagnetic configurations

# Comprehensive search with verbose output
./build/bin/amcheck -a -v POSCAR

# Search with custom tolerance
./build/bin/amcheck -a -t 1e-4 POSCAR
```

#### 3. Anomalous Hall Coefficient Analysis
```bash
# Standard AHC analysis
./build/bin/amcheck --ahc POSCAR

# AHC with verbose symmetry operations
./build/bin/amcheck --ahc -v POSCAR

# Combine AHC with custom parameters
./build/bin/amcheck --ahc -s 1e-4 -t 1e-4 POSCAR
```

#### 4. Advanced Usage Examples
```bash
# High precision comprehensive search
./build/bin/amcheck -a -v -s 1e-6 -t 1e-6 my_structure.vasp

# Quick check with standard precision
./build/bin/amcheck my_material.vasp

# Batch analysis (Linux/macOS)
for file in *.vasp; do
    echo "Analyzing $file..."
    ./build/bin/amcheck -a "$file"
done
```

### Interactive Mode Guide

#### Standard Mode Spin Assignment
When running standard analysis, you'll be prompted to assign spins to each atomic orbit:

```
Enter spin for orbit 1 (Mn): u    # spin-up
Enter spin for orbit 2 (O): d     # spin-down  
Enter spin for orbit 3 (F): n     # non-magnetic
```

**Accepted Inputs:**
- `u`, `U`, `up` → Spin-up (↑)
- `d`, `D`, `down` → Spin-down (↓)
- `n`, `N`, `none` → Non-magnetic (—)
- `nn`, `NN` → Mark entire orbit as non-magnetic

#### AHC Mode Magnetic Moments
For Anomalous Hall Coefficient analysis, enter magnetic moments in Cartesian coordinates:

```
Atom 1 (Fe): 2.5 0.0 0.5    # mx my mz components
Atom 2 (O):                 # Empty line for non-magnetic atom
Atom 3 (Ni): -1.2 0.0 0.8   # Negative values allowed
```

## Input Format & File Support

### VASP POSCAR Format
AMCheck C++ supports VASP POSCAR files with the following structure:

```
Title Line (Comment)
1.0                          # Scaling factor
8.0 0.0 0.0                  # Lattice vector a
0.0 8.0 0.0                  # Lattice vector b  
0.0 0.0 8.0                  # Lattice vector c
Fe O                         # Element symbols
2 4                          # Number of each element
Direct                       # Coordinate format (Direct/Cartesian)
0.0 0.0 0.0                  # Atomic positions
0.5 0.5 0.5
0.25 0.25 0.25
...
```

### Supported Features
- ✅ Direct and Cartesian coordinates
- ✅ Scaling factors (including negative values for volume scaling)
- ✅ Element symbols and atom counts
- ✅ Comments and blank lines
- ✅ Various POSCAR format variations
- ✅ Selective dynamics (if present, will be ignored)

### File Naming Conventions
- `POSCAR` (standard VASP format)
- `POSCAR.xyz` (with extension)
- `structure.vasp`
- Any filename ending with `.vasp`

## Output & Results

### Standard Analysis Output
```
=======================================================================
                      ALTERMAGNET ANALYSIS
=======================================================================
Processing: POSCAR
-----------------------------------------------------------------------
Structure loaded successfully!
Analyzing crystal symmetry...
Space Group: Pm-3m (221)
Number of symmetry operations: 48

Setting up magnetic configuration...
[Interactive spin assignment prompts]

Performing altermagnet detection...

=======================================================================
                         RESULT: ALTERMAGNET!
              Your material exhibits altermagnetic properties!
=======================================================================
```

### Comprehensive Search Results (NEW!)
When using the `-a` flag, results are automatically saved to a timestamped file:

**File: `POSCAR_amcheck_results_YYYYMMDD_HHMMSS.txt`**
```
# AMCheck C++ - Altermagnetic Spin Configurations
# Generated on: Jan 03 2025 14:30:25
# Structure: 12 atoms
# Total configurations tested: 531441
# Altermagnetic configurations found: 20280
# Success rate: 3.82%
# Tolerance: 0.001
#
# Atomic structure:
# Atom  1: Mn at ( 0.000000,  0.000000,  0.000000)
# Atom  2: Mn at ( 0.500000,  0.000000,  0.500000)
# Atom  3: Se at ( 0.368831,  0.368831,  0.368831)
# ...
#
# Format: ConfigID | Spin_Pattern | Detailed_Assignment
#         u = up, d = down, n = none
#

Config #    3244: d d u u d d d d u u u u | Mn(↓) Mn(↓) Mn(↑) Mn(↑) Mn(↓) Mn(↓) Se(↓) Se(↓) Se(↑) Se(↑) Se(↑) Se(↑)
Config #    3250: d u d u d d d d u u u u | Mn(↓) Mn(↑) Mn(↓) Mn(↑) Mn(↓) Mn(↓) Se(↓) Se(↓) Se(↑) Se(↑) Se(↑) Se(↑)
...
```

### Performance Summary
The comprehensive search provides detailed performance information:

```
Summary:
-----------------------------------------------------------------------
- Total configurations tested: 531441
- Altermagnetic configurations found: 20280  
- Success rate: 3.82%
- Results saved to: POSCAR_amcheck_results_20250103_143025.txt
=======================================================================
```

### AHC Mode Output
For Anomalous Hall Coefficient analysis:

```
Conductivity Tensor:
   [                                    ]
   [   0.3769600   -0.1229330   -0.3044320   ]
   [  -0.1229330   -0.2247420    0.2940065   ]
   [  -0.3044320    0.2940065    0.1709066   ]
   [                                    ]

Antisymmetric Part (Anomalous Hall Effect):
   [                                    ]
   [   0.0000000    0.0000000    0.0000000   ]
   [   0.0000000    0.0000000    0.0000000   ]
   [   0.0000000    0.0000000    0.0000000   ]
   [                                    ]

Hall Vector: [0.0000000, 0.0000000, 0.0000000]
```

## Performance & Optimization

### Computational Complexity
- **Search space**: 3^N configurations (N = number of atoms)
- **Memory usage**: O(N + found_configs)  
- **CPU scaling**: Linear scaling with number of cores
- **Time complexity**: O(3^N × symmetry_operations)

### System Requirements by Structure Size

| Structure | Atoms | Configurations | RAM | CPU Cores | Est. Time |
|-----------|-------|----------------|-----|-----------|-----------|
| Small | 4-8 | 81-6,561 | 2GB | 2+ | < 1 min |
| Medium | 8-12 | 6,561-531,441 | 4GB | 4+ | 1-10 min |
| Large | 12-16 | 531K-43M | 8GB | 8+ | 10-60 min |
| Very Large | 16+ | 43M+ | 16GB+ | 16+ | 1+ hours |

### Performance Tips
1. **Use comprehensive search (`-a`) wisely**: Only for structures with ≤16 atoms
2. **Optimize tolerance**: Higher tolerance = faster computation  
3. **Multi-core advantage**: Performance scales linearly with CPU cores
4. **Memory considerations**: Ensure sufficient RAM for large searches
5. **SSD storage**: Faster I/O for result file writing

### Benchmarks
*Example performance on Intel i7-12700K (8P+4E cores), 32GB RAM:*

- **8 atoms** (6,561 configs): ~5 seconds
- **12 atoms** (531K configs): ~2 minutes  
- **16 atoms** (43M configs): ~45 minutes

## Troubleshooting

### Common Build Issues

#### Missing Dependencies
```bash
# Error: Eigen3 not found
# Ubuntu/Debian:
sudo apt-get install libeigen3-dev

# MSYS2:
pacman -S mingw-w64-x86_64-eigen3

# Error: spglib not found or linking error (/usr/bin/ld: cannot find -lsymspg)
# Ubuntu/Debian (try in this order):
sudo apt-get update
sudo apt-get install libspglib-dev libspglib1
# OR if above doesn't work:
sudo apt-get install libsymspg-dev
# OR for newer systems:
sudo apt-get install spglib-dev

# Check what's actually installed:
dpkg -l | grep spg
ls /usr/lib/x86_64-linux-gnu/libspg*
ls /usr/include/spglib.h

# Fedora/RHEL:
sudo dnf install spglib-devel
# OR
sudo dnf install libsymspg-devel

# If none of the above work, build from source:
git clone https://github.com/spglib/spglib.git
cd spglib
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
sudo ldconfig

# MSYS2 (choose one based on your environment):
# For Clang environment:
pacman -S mingw-w64-clang-x86_64-spglib
# OR for UCRT environment:
pacman -S mingw-w64-ucrt-x86_64-spglib
```

#### CMake Issues
```bash
# Error: CMake version too old
# Download latest CMake from: https://cmake.org/download/

# Error: Compiler not found
# Install build-essential (Linux) or MinGW-w64 (MSYS2)
```

#### Build Configuration Issues
```bash
# Clear build cache
rm -rf build/
rm CMakeCache.txt

# Reconfigure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Runtime Issues

#### File Not Found
```bash
# Check file exists and has proper permissions
ls -la POSCAR
file POSCAR  # Check file type

# Try absolute path
./build/bin/amcheck /full/path/to/POSCAR
```

#### Memory Issues
```bash
# Monitor memory usage during large searches
htop  # Linux
Task Manager  # Windows

# Reduce search space or increase system RAM
```

#### Performance Issues
```bash
# Check CPU utilization
top -H  # Linux
Task Manager -> Performance -> CPU  # Windows

# Verify multithreading is working
./build/bin/amcheck -a -v POSCAR  # Look for "CPU cores available: X"
```

### Standalone Build Issues

#### Static Linking Problems
```bash
# Error: Cannot find static libraries
# Solution: Install static development packages
# Ubuntu/Debian:
sudo apt-get install libc6-dev libstdc++-dev-static

# MSYS2: Usually included by default
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gcc-libs
```

#### Executable Still Has Dependencies
```bash
# Check what's linked
ldd build/bin/amcheck  # Linux
ldd build/bin/amcheck.exe  # Windows/MSYS2

# If unwanted dependencies appear, try fully static build
./build.sh --fully-static  # Linux
./build_msys2.sh --fully-static  # Windows

# Alternative: Force static linking
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_FULLY_STATIC=ON
```

#### Large Executable Size
```bash
# Static executables are larger - this is normal
# Typical sizes:
# - Dynamic build: 500KB - 2MB
# - Static build: 2MB - 10MB
# - Fully static: 5MB - 20MB

# To reduce size, use standard static (not fully static)
./build.sh  # Default static linking
```

#### Missing Static Libraries
```bash
# Error: libspglib.a not found
# MSYS2: Install static versions
pacman -S mingw-w64-x86_64-spglib-static  # If available

# Ubuntu/Debian: Build from source if static library missing
# Download spglib source and build with -DBUILD_SHARED_LIBS=OFF
```

### Platform-Specific Issues

#### Windows/MSYS2
- **Issue**: "command not found"
  - **Solution**: Ensure using MinGW64 terminal, not MSYS2 terminal
- **Issue**: Path problems
  - **Solution**: Use forward slashes `/` in MSYS2 environment
- **Issue**: Antivirus blocking
  - **Solution**: Add build directory to antivirus exclusions

#### Linux
- **Issue**: Permission denied
  - **Solution**: `chmod +x build.sh clean.sh`
- **Issue**: Missing shared libraries
  - **Solution**: `sudo ldconfig` or check `LD_LIBRARY_PATH`

#### macOS
- **Issue**: Developer tools not found
  - **Solution**: `xcode-select --install`
- **Issue**: Homebrew not in PATH
  - **Solution**: `export PATH="/opt/homebrew/bin:$PATH"`

## Advanced Features

### Debug Build
For development and troubleshooting:

```bash
mkdir build-debug && cd build-debug
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run with debugging symbols
gdb ./bin/amcheck
valgrind --tool=memcheck ./bin/amcheck POSCAR
```

### Custom Installation
```bash
# Install to custom location
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/amcheck
make install

# Add to PATH
export PATH="/usr/local/amcheck/bin:$PATH"
```

## Current Limitations & Future Improvements

### Current Status ✅
- ✅ Complete spglib integration for space group detection
- ✅ VASP POSCAR file support with robust parsing
- ✅ Multithreaded comprehensive spin configuration search
- ✅ Cross-platform compilation (Linux, Windows MSYS2, macOS)
- ✅ Interactive and batch processing modes
- ✅ Detailed output file generation
- ✅ Performance optimization for large structures

### Potential Enhancements 🔮
- 🔮 Additional file format support (CIF, XYZ, COOT, etc.)
- 🔮 Magnetic space group analysis integration
- 🔮 Primitive cell detection and transformation
- 🔮 Web interface for online analysis
- 🔮 Python bindings for integration with existing workflows
- 🔮 GPU acceleration for extremely large structures
- 🔮 Machine learning-assisted configuration prediction

## License & Citation

### License
This project is licensed under the **BSD 3-Clause License**. See the LICENSE file for details.

### Citation
If you use AMCheck C++ in your research, please cite:

```bibtex
@software{amcheck_cpp_2025,
  title={AMCheck C++: High-Performance Altermagnet Detection Tool},
  author={Ali, Nasir and Faisal, Shah},
  supervisor={Rahman, Gul},
  year={2025},
  institution={Department of Physics, Quaid-i-Azam University},
  url={https://github.com/nasirxo/amcheckcpp},
  version={1.0.0},
  note={Cross-platform tool for detecting altermagnetic materials}
}
```

### Acknowledgments
- **Quaid-i-Azam University** for providing academic support and resources
- **Prof. Dr. Gul Rahman** for supervision, guidance, and theoretical insights
- **spglib development team** for excellent crystallographic analysis tools
- **Eigen3 team** for the high-performance linear algebra library
- **Open source community** for various tools and libraries used in this project

## Contact & Support

### Primary Contacts
- **Nasir Ali**: [nasiraliphy@gmail.com](mailto:nasiraliphy@gmail.com) - Technical inquiries, bug reports
- **Shah Faisal**: [shahf8885@gmail.com](mailto:shahf8885@gmail.com) - Feature requests, collaboration
- **Prof. Dr. Gul Rahman**: [gulrahman@qau.edu.pk](mailto:gulrahman@qau.edu.pk) - Academic collaboration, research

### Getting Help
1. **Documentation**: Read this README thoroughly first
2. **GitHub Issues**: Report bugs and request features at [GitHub Issues](https://github.com/nasirxo/amcheckcpp/issues)
3. **GitHub Discussions**: Ask questions at [GitHub Discussions](https://github.com/nasirxo/amcheckcpp/discussions)  
4. **Email Support**: Contact authors directly for urgent issues or academic collaboration

### Contributing
We welcome contributions! Please:
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes with appropriate tests
4. Commit your changes (`git commit -m 'Add amazing feature'`)
5. Push to the branch (`git push origin feature/amazing-feature`)
6. Open a Pull Request

---

**© 2025 Nasir Ali, Shah Faisal, Quaid-i-Azam University. All Rights Reserved.**

*AMCheck C++ - Advancing computational materials science through high-performance altermagnet detection.*

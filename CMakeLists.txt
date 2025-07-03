cmake_minimum_required(VERSION 3.15)
project(amcheck_cpp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LINUX TRUE)
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
endif()

# Find threading library
find_package(Threads REQUIRED)

# Eigen3 detection strategy based on platform
if(MSYS OR MINGW OR PLATFORM_WINDOWS)
    # MSYS2/MinGW specific settings
    if(DEFINED ENV{MINGW_PREFIX})
        set(ENV{PKG_CONFIG_PATH} "$ENV{MINGW_PREFIX}/lib/pkgconfig")
        include_directories("$ENV{MINGW_PREFIX}/include")
        link_directories("$ENV{MINGW_PREFIX}/lib")
        message(STATUS "MSYS2/MinGW environment detected: $ENV{MINGW_PREFIX}")
    endif()
    
    # Try pkg-config first
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(EIGEN3 eigen3)
    endif()
    
    if(EIGEN3_FOUND)
        include_directories(${EIGEN3_INCLUDE_DIRS})
        message(STATUS "Found Eigen3 via pkg-config: ${EIGEN3_INCLUDE_DIRS}")
    else()
        # Manual search in common Windows/MSYS2 locations
        find_path(EIGEN3_INCLUDE_DIR 
            NAMES Eigen/Dense
            PATHS 
                "$ENV{MINGW_PREFIX}/include/eigen3" 
                "$ENV{MINGW_PREFIX}/include"
                "/mingw64/include/eigen3"
                "/mingw32/include/eigen3"
                "/usr/include/eigen3"
                "C:/vcpkg/installed/x64-windows/include/eigen3"
                "C:/Program Files/eigen3/include"
            NO_DEFAULT_PATH
        )
        if(EIGEN3_INCLUDE_DIR)
            include_directories(${EIGEN3_INCLUDE_DIR})
            message(STATUS "Found Eigen3 manually at: ${EIGEN3_INCLUDE_DIR}")
        else()
            # Fallback to standard find_package
            find_package(Eigen3 QUIET)
            if(NOT Eigen3_FOUND)
                message(WARNING "Eigen3 not found. Please install with: pacman -S mingw-w64-x86_64-eigen3")
            endif()
        endif()
    endif()
    
elseif(PLATFORM_LINUX)
    # Linux-specific Eigen3 detection
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(EIGEN3 eigen3)
    endif()
    
    if(EIGEN3_FOUND)
        include_directories(${EIGEN3_INCLUDE_DIRS})
        message(STATUS "Found Eigen3 via pkg-config: ${EIGEN3_INCLUDE_DIRS}")
    else()
        # Try standard locations
        find_path(EIGEN3_INCLUDE_DIR 
            NAMES Eigen/Dense
            PATHS 
                "/usr/include/eigen3"
                "/usr/local/include/eigen3"
                "/opt/local/include/eigen3"
        )
        if(EIGEN3_INCLUDE_DIR)
            include_directories(${EIGEN3_INCLUDE_DIR})
            message(STATUS "Found Eigen3 at: ${EIGEN3_INCLUDE_DIR}")
        else()
            find_package(Eigen3 QUIET)
            if(NOT Eigen3_FOUND)
                message(FATAL_ERROR "Eigen3 not found. Install with: sudo apt-get install libeigen3-dev")
            endif()
        endif()
    endif()
    
else()
    # macOS and other systems
    find_package(Eigen3 REQUIRED)
endif()

# spglib detection
message(STATUS "Searching for spglib...")

# First try pkg-config
if(PkgConfig_FOUND)
    pkg_check_modules(SPGLIB symspg)
    if(NOT SPGLIB_FOUND)
        pkg_check_modules(SPGLIB spglib)
    endif()
endif()

if(NOT SPGLIB_FOUND)
    # Manual search for spglib
    find_path(SPGLIB_INCLUDE_DIR 
        NAMES spglib.h
        PATHS 
            "$ENV{MINGW_PREFIX}/include"
            "/usr/include" 
            "/usr/local/include"
            "/opt/homebrew/include"
    )
    
    # Try multiple library name variations
    find_library(SPGLIB_LIBRARY 
        NAMES symspg spg spglib libsymspg libspg libspglib
        PATHS 
            "$ENV{MINGW_PREFIX}/lib"
            "/usr/lib" 
            "/usr/lib/x86_64-linux-gnu"
            "/usr/local/lib"
            "/opt/homebrew/lib"
    )
    
    if(SPGLIB_INCLUDE_DIR AND SPGLIB_LIBRARY)
        set(SPGLIB_FOUND TRUE)
        set(SPGLIB_INCLUDE_DIRS ${SPGLIB_INCLUDE_DIR})
        set(SPGLIB_LIBRARIES ${SPGLIB_LIBRARY})
        message(STATUS "Found spglib manually: ${SPGLIB_INCLUDE_DIR}")
    elif(SPGLIB_INCLUDE_DIR)
        # Header found but no library - try to link anyway with default name
        set(SPGLIB_FOUND TRUE)
        set(SPGLIB_INCLUDE_DIRS ${SPGLIB_INCLUDE_DIR})
        set(SPGLIB_LIBRARIES "symspg")  # Try default library name
        message(STATUS "Found spglib header: ${SPGLIB_INCLUDE_DIR}")
        message(STATUS "Using default library name: symspg")
    endif()
endif()

if(SPGLIB_FOUND)
    message(STATUS "✅ spglib found and will be linked")
    add_definitions(-DHAVE_SPGLIB)
    include_directories(${SPGLIB_INCLUDE_DIRS})
else()
    message(WARNING "spglib not found. Space group detection will be limited.")
    message(STATUS "To install spglib:")
    if(MSYS OR MINGW)
        message(STATUS "  MSYS2: pacman -S mingw-w64-x86_64-spglib")
    elseif(PLATFORM_LINUX)
        message(STATUS "  Ubuntu/Debian: sudo apt-get install libsymspg-dev")
        message(STATUS "  Fedora: sudo dnf install spglib-devel")
    elseif(PLATFORM_MACOS)
        message(STATUS "  macOS: brew install spglib")
    endif()
endif()

# Create the main executable
add_executable(amcheck
    src/main.cpp
    src/amcheck.cpp
    src/crystal_structure.cpp
    src/symmetry_operations.cpp
    src/spins.cpp
    src/utils.cpp
)

target_include_directories(amcheck PRIVATE
    include
)

# Link Eigen3 if found
if(TARGET Eigen3::Eigen)
    target_link_libraries(amcheck Eigen3::Eigen)
endif()

# Link spglib if found
if(SPGLIB_FOUND)
    target_link_libraries(amcheck ${SPGLIB_LIBRARIES})
endif()

# Link threading library
target_link_libraries(amcheck Threads::Threads)

# Set compiler flags for standalone binaries
if(MSVC)
    target_compile_options(amcheck PRIVATE /W4)
    # Static linking for MSVC
    set_property(TARGET amcheck PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
else()
    target_compile_options(amcheck PRIVATE -Wall -Wextra -Wpedantic -O2)
    
    # Enable static linking for standalone binaries
    if(MSYS OR MINGW OR PLATFORM_WINDOWS)
        # MSYS2/MinGW static linking
        target_compile_options(amcheck PRIVATE -static-libgcc -static-libstdc++)
        target_link_options(amcheck PRIVATE -static -static-libgcc -static-libstdc++)
        # Link static runtime libraries
        target_link_libraries(amcheck -static)
    elseif(PLATFORM_LINUX)
        # Linux static linking (optional, can be enabled with BUILD_STATIC)
        option(BUILD_STATIC "Build static executable" ON)
        if(BUILD_STATIC)
            target_link_options(amcheck PRIVATE -static-libgcc -static-libstdc++)
            # Optionally link everything statically
            option(BUILD_FULLY_STATIC "Build fully static executable" OFF)
            if(BUILD_FULLY_STATIC)
                target_link_options(amcheck PRIVATE -static)
            endif()
        endif()
    endif()
endif()

# Set output directory
set_target_properties(amcheck PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Installation
install(TARGETS amcheck 
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Create a simple wrapper script for easier execution
if(MSYS OR MINGW)
    # Install the launcher script
    install(PROGRAMS amcheck
        DESTINATION bin
        COMPONENT Runtime
    )
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")

if(MSYS OR MINGW)
    message(STATUS "MSYS2/MinGW environment detected")
    message(STATUS "MINGW_PREFIX: $ENV{MINGW_PREFIX}")
endif()

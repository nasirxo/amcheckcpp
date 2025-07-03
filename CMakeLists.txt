cmake_minimum_required(VERSION 3.5)
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

# CUDA support (optional)
option(ENABLE_CUDA "Enable CUDA GPU acceleration" ON)

if(ENABLE_CUDA)
    # Check if CUDA compiler is available before enabling the language
    find_program(NVCC_EXECUTABLE nvcc)
    
    if(NVCC_EXECUTABLE)
        enable_language(CUDA)
        find_package(CUDA QUIET)
        
        if(CUDA_FOUND)
            # Check CUDA version compatibility - support CUDA 8.0+
            if(CUDA_VERSION VERSION_LESS "8.0")
                message(STATUS "CUDA ${CUDA_VERSION} is too old (requires 8.0+) - CUDA disabled")
                set(HAVE_CUDA FALSE)
            else()
                # For CUDA 8.0-10.x, try to find CUDAToolkit, but continue without it if not found
                find_package(CUDAToolkit QUIET)
                
                set(HAVE_CUDA TRUE)
                add_definitions(-DHAVE_CUDA)
                message(STATUS "CUDA found: ${CUDA_VERSION}")
                
                if(CUDAToolkit_FOUND)
                    message(STATUS "CUDA Toolkit found: ${CUDAToolkit_VERSION}")
                else()
                    message(STATUS "CUDA Toolkit package not found, using basic CUDA support")
                endif()
                
                # Set CUDA flags based on version - older CUDA versions need lower standards
                if(CUDA_VERSION VERSION_GREATER_EQUAL "11.0")
                    set(CMAKE_CUDA_STANDARD 17)
                    message(STATUS "CUDA ${CUDA_VERSION} supports C++17")
                elseif(CUDA_VERSION VERSION_GREATER_EQUAL "9.0")
                    set(CMAKE_CUDA_STANDARD 14)
                    message(STATUS "CUDA ${CUDA_VERSION} - using C++14 for CUDA code")
                else()
                    # CUDA 8.0 - use C++11
                    set(CMAKE_CUDA_STANDARD 11)
                    message(STATUS "CUDA ${CUDA_VERSION} is old - using C++11 for CUDA code")
                endif()
                set(CMAKE_CUDA_STANDARD_REQUIRED ON)
                set(CMAKE_CUDA_SEPARABLE_COMPILATION ON)
                
                # CUDA architecture detection - adjust for older CUDA
                if(NOT DEFINED CMAKE_CUDA_ARCHITECTURES)
                    if(CUDA_VERSION VERSION_GREATER_EQUAL "9.0")
                        set(CMAKE_CUDA_ARCHITECTURES "50;61;75")  # Remove 86 for older CUDA
                    else()
                        set(CMAKE_CUDA_ARCHITECTURES "35;50;61")  # CUDA 8.0 compatible architectures
                    endif()
                    message(STATUS "Using CUDA architectures: ${CMAKE_CUDA_ARCHITECTURES}")
                endif()
                
                # Set CUDA compilation flags separately from C++ flags
                if(CUDA_VERSION VERSION_GREATER_EQUAL "9.0")
                    set(CMAKE_CUDA_FLAGS_RELEASE "-O3 --use_fast_math")
                else()
                    # CUDA 8.0 - simpler optimization flags
                    set(CMAKE_CUDA_FLAGS_RELEASE "-O2")
                endif()
                set(CMAKE_CUDA_FLAGS_DEBUG "-g -G")
                
                # Add relaxed constexpr for all CUDA versions that support it
                if(CUDA_VERSION VERSION_GREATER_EQUAL "9.0")
                    set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-relaxed-constexpr")
                endif()
                
            endif()
        else()
            message(STATUS "CUDA compiler found but CUDA not found - CUDA disabled")
            set(HAVE_CUDA FALSE)
        endif()
    else()
        message(STATUS "NVCC compiler not found - CUDA disabled")
        set(HAVE_CUDA FALSE)
    endif()
else()
    message(STATUS "CUDA support disabled by user")
    set(HAVE_CUDA FALSE)
endif()

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

# Manual spglib linking for MSYS2
message(STATUS "Setting up manual spglib linking...")

# Force spglib integration by directly linking known files
if(MSYS OR MINGW OR PLATFORM_WINDOWS)
    # We know spglib exists in clang64, just use it directly
    set(SPGLIB_FOUND TRUE)
    set(SPGLIB_INCLUDE_DIRS "/clang64/include")
    set(SPGLIB_LIBRARIES "/clang64/lib/libsymspg.dll.a")
    set(SPGLIB_LIBRARY_DIRS "/clang64/lib")
    set(SPGLIB_DLL_PATH "/clang64/bin/libsymspg-2.dll")
    
    message(STATUS "✅ Using direct spglib linking (MSYS2)")
    message(STATUS "  Header: ${SPGLIB_INCLUDE_DIRS}/spglib.h")
    message(STATUS "  Library: ${SPGLIB_LIBRARIES}")
    message(STATUS "  DLL: ${SPGLIB_DLL_PATH}")
    
    # Enable spglib compilation
    add_definitions(-DHAVE_SPGLIB)
    include_directories(${SPGLIB_INCLUDE_DIRS})
    
else()
    # For Linux, try standard detection with multiple search paths
    find_path(SPGLIB_INCLUDE_DIR 
        NAMES spglib.h 
        PATHS 
            "/usr/include"
            "/usr/include/spglib"
            "/usr/local/include"
            "/usr/local/include/spglib"
            "/opt/local/include"
            "/opt/local/include/spglib"
    )
    
    # Also check for the spglib subdirectory case
    if(NOT SPGLIB_INCLUDE_DIR)
        find_path(SPGLIB_INCLUDE_DIR 
            NAMES spglib/spglib.h 
            PATHS 
                "/usr/include"
                "/usr/local/include"
                "/opt/local/include"
        )
        if(SPGLIB_INCLUDE_DIR)
            # If found in subdirectory, include the parent directory
            message(STATUS "Found spglib.h in subdirectory: ${SPGLIB_INCLUDE_DIR}/spglib/spglib.h")
        endif()
    endif()
    
    find_library(SPGLIB_LIBRARY 
        NAMES symspg spglib libsymspg libspglib
        PATHS 
            "/usr/lib/x86_64-linux-gnu"
            "/usr/lib"
            "/usr/lib64"
            "/usr/local/lib"
            "/opt/local/lib"
    )
    
    if(SPGLIB_INCLUDE_DIR AND SPGLIB_LIBRARY)
        set(SPGLIB_FOUND TRUE)
        set(SPGLIB_INCLUDE_DIRS ${SPGLIB_INCLUDE_DIR})
        set(SPGLIB_LIBRARIES ${SPGLIB_LIBRARY})
        add_definitions(-DHAVE_SPGLIB)
        include_directories(${SPGLIB_INCLUDE_DIRS})
        message(STATUS "✅ Found spglib on Linux:")
        message(STATUS "  Header: ${SPGLIB_INCLUDE_DIR}")
        message(STATUS "  Library: ${SPGLIB_LIBRARY}")
    else()
        set(SPGLIB_FOUND FALSE)
        message(STATUS "❌ spglib not found on Linux - building without space group detection")
        message(STATUS "  Header search paths checked: /usr/include, /usr/include/spglib, /usr/local/include")
        message(STATUS "  Library search paths checked: /usr/lib*, /usr/local/lib")
    endif()
endif()

# Create the main executable
# Create source lists
set(AMCHECK_SOURCES
    src/main.cpp
    src/amcheck.cpp
    src/crystal_structure.cpp
    src/symmetry_operations.cpp
    src/spins.cpp
    src/utils.cpp
)

# Add CUDA sources if available
if(HAVE_CUDA)
    list(APPEND AMCHECK_SOURCES src/cuda_accelerator.cpp)
    set_property(SOURCE src/cuda_accelerator.cpp PROPERTY LANGUAGE CUDA)
    
    # Set CUDA-specific compilation flags to avoid conflicts - removed duplicate optimization flags
endif()

add_executable(amcheck ${AMCHECK_SOURCES})

target_include_directories(amcheck PRIVATE
    include
)

# Link Eigen3 if found
if(TARGET Eigen3::Eigen)
    target_link_libraries(amcheck Eigen3::Eigen)
endif()

# Link CUDA if available
if(HAVE_CUDA)
    # Handle different CUDA linking methods based on version
    if(CUDAToolkit_FOUND)
        # Modern CUDA linking (CUDA 10.1+)
        target_link_libraries(amcheck CUDA::cudart CUDA::curand)
    else()
        # Legacy CUDA linking (CUDA 8.0-10.0)
        target_link_libraries(amcheck ${CUDA_LIBRARIES} ${CUDA_curand_LIBRARY})
        target_include_directories(amcheck PRIVATE ${CUDA_INCLUDE_DIRS})
    endif()
    
    set_target_properties(amcheck PROPERTIES 
        CUDA_SEPARABLE_COMPILATION ON
    )
    
    # Only set CUDA_RESOLVE_DEVICE_SYMBOLS for newer CMake/CUDA versions
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.18")
        set_target_properties(amcheck PROPERTIES CUDA_RESOLVE_DEVICE_SYMBOLS ON)
    endif()
    
    message(STATUS "✅ CUDA support enabled")
else()
    message(STATUS "ℹ️  Building without CUDA support")
endif()

# Link spglib if found
if(SPGLIB_FOUND)
    # Link the spglib library
    target_link_libraries(amcheck ${SPGLIB_LIBRARIES})
    message(STATUS "✅ Linking amcheck with spglib: ${SPGLIB_LIBRARIES}")
    
    # For MSYS2, copy the required DLLs to the output directory for standalone execution
    if(MSYS OR MINGW OR PLATFORM_WINDOWS)
        # Copy spglib DLL
        if(EXISTS "/clang64/bin/libsymspg-2.dll")
            add_custom_command(TARGET amcheck POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "/clang64/bin/libsymspg-2.dll"
                    "$<TARGET_FILE_DIR:amcheck>/libsymspg-2.dll"
                COMMENT "Copying spglib DLL for standalone execution"
            )
            message(STATUS "✅ Will copy spglib DLL: libsymspg-2.dll")
        endif()
        
        # Copy OpenMP DLL (required by spglib)
        if(EXISTS "/clang64/bin/libomp.dll")
            add_custom_command(TARGET amcheck POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "/clang64/bin/libomp.dll"
                    "$<TARGET_FILE_DIR:amcheck>/libomp.dll"
                COMMENT "Copying OpenMP DLL for spglib dependency"
            )
            message(STATUS "✅ Will copy OpenMP DLL: libomp.dll")
        endif()
    endif()
else()
    message(STATUS "❌ Building without spglib integration")
endif()

# Link threading library
target_link_libraries(amcheck Threads::Threads)

# Set compiler flags for standalone binaries
if(MSVC)
    target_compile_options(amcheck PRIVATE /W4)
    # Static linking for MSVC
    set_property(TARGET amcheck PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
else()
    # Set different compile options for C++ only (CUDA flags set globally above)
    target_compile_options(amcheck PRIVATE 
        $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic -O2>
    )
    
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
                # Check if static libraries are available
                if(SPGLIB_FOUND)
                    # Try to find static version of spglib
                    find_library(SPGLIB_STATIC_LIBRARY 
                        NAMES libsymspg.a libspglib.a
                        PATHS 
                            "/usr/lib/x86_64-linux-gnu"
                            "/usr/lib"
                            "/usr/lib64"
                            "/usr/local/lib"
                    )
                    
                    if(NOT SPGLIB_STATIC_LIBRARY)
                        message(WARNING "Static spglib library not found. Cannot build fully static.")
                        message(STATUS "Available libraries are shared only: ${SPGLIB_LIBRARIES}")
                        message(STATUS "Building with dynamic spglib linking instead.")
                        message(STATUS "Install libsymspg-dev with static libraries or build spglib from source with -DBUILD_STATIC=ON")
                        
                        # Use dynamic linking for spglib, static for runtime
                        target_link_options(amcheck PRIVATE -Wl,-Bstatic -static-libgcc -static-libstdc++ -Wl,-Bdynamic)
                    else()
                        message(STATUS "Found static spglib: ${SPGLIB_STATIC_LIBRARY}")
                        target_link_options(amcheck PRIVATE -static)
                    endif()
                else()
                    # No spglib, full static is possible
                    target_link_options(amcheck PRIVATE -static)
                endif()
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

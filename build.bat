@echo off
echo Building AMCheck C++...

if not exist build mkdir build
cd build

echo Configuring with CMake...
cmake ..

if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b 1
)

echo Building...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build successful!
echo Executable location: build\Release\amcheck.exe (or build\amcheck.exe depending on generator)
pause

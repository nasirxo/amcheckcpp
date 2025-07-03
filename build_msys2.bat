@echo off
REM Build script for MSYS2 on Windows
REM This script launches MSYS2 and runs the build

echo Building AMCheck with MSYS2...

REM Check if MSYS2 is installed in common locations
set MSYS2_PATH=
if exist "C:\msys64\usr\bin\bash.exe" (
    set MSYS2_PATH=C:\msys64
) else if exist "C:\msys2\usr\bin\bash.exe" (
    set MSYS2_PATH=C:\msys2
) else if exist "%USERPROFILE%\msys64\usr\bin\bash.exe" (
    set MSYS2_PATH=%USERPROFILE%\msys64
) else (
    echo Error: MSYS2 not found in common locations.
    echo Please install MSYS2 from https://www.msys2.org/
    echo Or modify this script to point to your MSYS2 installation.
    pause
    exit /b 1
)

echo Found MSYS2 at: %MSYS2_PATH%

REM Launch MSYS2 MinGW64 environment and run build script
echo Launching MSYS2 build...
"%MSYS2_PATH%\usr\bin\bash.exe" -l -c "cd '%~dp0' && ./build_msys2.sh"

if %ERRORLEVEL% equ 0 (
    echo Build completed successfully!
) else (
    echo Build failed with error code %ERRORLEVEL%
)

pause

@echo off
REM Script to prepare compiled binaries for distribution on Windows
REM This creates a compiled_binaries directory structure and copies built executables

setlocal enabledelayedexpansion

REM Colors for output (using simple text since Windows CMD has limited color support)
set "INFO_PREFIX=[INFO]"
set "SUCCESS_PREFIX=[SUCCESS]"
set "WARNING_PREFIX=[WARNING]"
set "ERROR_PREFIX=[ERROR]"

echo ==================================================================
echo          AMCheck C++ Compiled Binaries Preparation (Windows)
echo ==================================================================
echo.

set "PLATFORM=windows"
set "EXECUTABLE_NAME=amcheck.exe"

echo %INFO_PREFIX% Platform detected: %PLATFORM%

REM Create directory structure
echo %INFO_PREFIX% Creating compiled_binaries directory structure...
mkdir compiled_binaries 2>nul
mkdir compiled_binaries\%PLATFORM% 2>nul
mkdir compiled_binaries\windows 2>nul
mkdir compiled_binaries\win64 2>nul
mkdir compiled_binaries\win32 2>nul

REM Find executable
set "EXECUTABLE="
if exist "build\bin\%EXECUTABLE_NAME%" (
    set "EXECUTABLE=build\bin\%EXECUTABLE_NAME%"
) else if exist "bin\%EXECUTABLE_NAME%" (
    set "EXECUTABLE=bin\%EXECUTABLE_NAME%"
) else if exist "%EXECUTABLE_NAME%" (
    set "EXECUTABLE=%EXECUTABLE_NAME%"
) else (
    echo %ERROR_PREFIX% Executable %EXECUTABLE_NAME% not found!
    echo %ERROR_PREFIX% Please build the project first using: build_msys2.bat
    pause
    exit /b 1
)

echo %SUCCESS_PREFIX% Found executable: %EXECUTABLE%

REM Get file size
for %%A in ("%EXECUTABLE%") do set "FILE_SIZE=%%~zA"
set /a "FILE_SIZE_MB=!FILE_SIZE!/1024/1024"
echo %INFO_PREFIX% File size: !FILE_SIZE_MB! MB

REM Copy to appropriate directories
echo %INFO_PREFIX% Copying executable to compiled_binaries directories...

REM Copy to platform-specific directory
copy "%EXECUTABLE%" "compiled_binaries\%PLATFORM%\%EXECUTABLE_NAME%" >nul
echo %SUCCESS_PREFIX% Copied to compiled_binaries\%PLATFORM%\

REM Copy to root of compiled_binaries for convenience
copy "%EXECUTABLE%" "compiled_binaries\%EXECUTABLE_NAME%" >nul
echo %SUCCESS_PREFIX% Copied to compiled_binaries\

REM Platform-specific additional copies
copy "%EXECUTABLE%" "compiled_binaries\windows\%EXECUTABLE_NAME%" >nul
copy "%EXECUTABLE%" "compiled_binaries\win64\%EXECUTABLE_NAME%" >nul
echo %SUCCESS_PREFIX% Copied to windows and win64 directories

REM Create info file
echo %INFO_PREFIX% Creating binary information file...
(
echo AMCheck C++ Compiled Binaries
echo ============================
echo.
echo Build Information:
echo - Platform: %PLATFORM%
echo - Build Date: %DATE% %TIME%
echo - File Size: !FILE_SIZE_MB! MB
echo.
echo Directory Structure:
echo - compiled_binaries\%EXECUTABLE_NAME%           # Generic binary
echo - compiled_binaries\%PLATFORM%\%EXECUTABLE_NAME% # Platform-specific
echo - compiled_binaries\windows\%EXECUTABLE_NAME%   # Windows-specific
echo - compiled_binaries\win64\%EXECUTABLE_NAME%     # Win64-specific
echo.
echo Installation:
echo Windows:     install_windows.bat
echo Linux/macOS: ./install.sh
echo.
echo The installation scripts will automatically detect and use these
echo pre-compiled binaries if no built executable is found.
echo.
echo Usage:
echo %EXECUTABLE_NAME% --help
echo %EXECUTABLE_NAME% POSCAR                    # Basic analysis
echo %EXECUTABLE_NAME% -b BAND.dat               # Band analysis
echo %EXECUTABLE_NAME% -a POSCAR                 # Comprehensive search
echo %EXECUTABLE_NAME% --ahc POSCAR              # Anomalous Hall analysis
) > compiled_binaries\README.txt

REM List created files
echo %INFO_PREFIX% Created compiled binaries:
dir /s compiled_binaries\%EXECUTABLE_NAME%

echo.
echo %SUCCESS_PREFIX% Compiled binaries prepared successfully!
echo %INFO_PREFIX% Directory: compiled_binaries\
echo %INFO_PREFIX% Installation scripts will now automatically detect these binaries

echo.
echo ==================================================================
echo             Ready for Distribution!
echo ==================================================================

pause

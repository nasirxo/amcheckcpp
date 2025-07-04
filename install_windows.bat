@echo off
REM AMCheck C++ Installation Script for Windows
REM This script copies the executable to a directory in PATH for global access

setlocal enabledelayedexpansion

REM Colors for output (using simple text since Windows CMD has limited color support)
set "INFO_PREFIX=[INFO]"
set "SUCCESS_PREFIX=[SUCCESS]"
set "WARNING_PREFIX=[WARNING]"
set "ERROR_PREFIX=[ERROR]"

echo ==================================================================
echo                 AMCheck C++ Installation Script for Windows
echo ==================================================================
echo.

REM Check for uninstall option
if "%1"=="--uninstall" goto :uninstall
if "%1"=="-u" goto :uninstall
if "%1"=="--help" goto :show_help
if "%1"=="-h" goto :show_help

REM Find the executable
set "EXECUTABLE="
if exist "build\bin\amcheck.exe" (
    set "EXECUTABLE=build\bin\amcheck.exe"
) else if exist "bin\amcheck.exe" (
    set "EXECUTABLE=bin\amcheck.exe"
) else if exist "amcheck.exe" (
    set "EXECUTABLE=amcheck.exe"
) else if exist "compiled_binaries\amcheck.exe" (
    set "EXECUTABLE=compiled_binaries\amcheck.exe"
    echo %INFO_PREFIX% Using pre-compiled binary from compiled_binaries\
) else if exist "compiled_binaries\windows\amcheck.exe" (
    set "EXECUTABLE=compiled_binaries\windows\amcheck.exe"
    echo %INFO_PREFIX% Using pre-compiled Windows binary from compiled_binaries\windows\
) else if exist "compiled_binaries\win64\amcheck.exe" (
    set "EXECUTABLE=compiled_binaries\win64\amcheck.exe"
    echo %INFO_PREFIX% Using pre-compiled Win64 binary from compiled_binaries\win64\
) else (
    echo %ERROR_PREFIX% AMCheck executable not found!
    echo %ERROR_PREFIX% Please build the project first using: build_msys2.bat
    echo %ERROR_PREFIX% Expected locations: build\bin\amcheck.exe, bin\amcheck.exe, amcheck.exe,
    echo %ERROR_PREFIX%                    or compiled_binaries\[windows^|win64]\amcheck.exe
    pause
    exit /b 1
)

echo %SUCCESS_PREFIX% Found executable: %EXECUTABLE%

REM Check if AMCheck already exists in system
where amcheck >nul 2>&1
if !errorLevel! == 0 (
    echo %WARNING_PREFIX% AMCheck already installed in system PATH
    for /f "tokens=*" %%A in ('where amcheck 2^>nul') do (
        echo %INFO_PREFIX% Existing installation: %%A
    )
    echo.
    set /p "REPLACE=Do you want to replace the existing installation? [y/N]: "
    if /i not "!REPLACE!"=="y" (
        echo %INFO_PREFIX% Keeping existing installation
        pause
        exit /b 0
    )
)

REM Check if compiled_binaries directory exists and list available binaries
if exist "compiled_binaries" (
    echo %INFO_PREFIX% Found compiled_binaries directory
    if exist "compiled_binaries\*.exe" (
        echo %INFO_PREFIX% Available pre-compiled binaries:
        dir /b compiled_binaries\*.exe 2>nul
    )
    if exist "compiled_binaries\windows\*.exe" (
        echo %INFO_PREFIX% Available Windows binaries:
        dir /b compiled_binaries\windows\*.exe 2>nul
    )
    if exist "compiled_binaries\win64\*.exe" (
        echo %INFO_PREFIX% Available Win64 binaries:
        dir /b compiled_binaries\win64\*.exe 2>nul
    )
    echo.
)

REM Get file size
for %%A in ("%EXECUTABLE%") do set "FILE_SIZE=%%~zA"
set /a "FILE_SIZE_MB=!FILE_SIZE!/1024/1024"
echo %INFO_PREFIX% Executable size: !FILE_SIZE_MB! MB

REM Determine installation directory
set "INSTALL_DIR="
set "INSTALL_TYPE="

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% == 0 (
    REM Running as admin - install to Program Files
    set "INSTALL_DIR=C:\Program Files\AMCheck"
    set "INSTALL_TYPE=system-wide (administrator)"
) else (
    REM Not admin - install to user directory
    set "INSTALL_DIR=%USERPROFILE%\AppData\Local\AMCheck"
    set "INSTALL_TYPE=user-local"
)

echo %INFO_PREFIX% Installation type: !INSTALL_TYPE!
echo %INFO_PREFIX% Installation directory: !INSTALL_DIR!
echo.

REM Ask for confirmation
set /p "CONFIRM=Proceed with installation? [Y/n]: "
if /i "!CONFIRM!"=="n" (
    echo %INFO_PREFIX% Installation cancelled
    pause
    exit /b 0
)

REM Create installation directory
if not exist "!INSTALL_DIR!" (
    echo %INFO_PREFIX% Creating directory: !INSTALL_DIR!
    mkdir "!INSTALL_DIR!" 2>nul
    if !errorLevel! neq 0 (
        echo %ERROR_PREFIX% Failed to create directory: !INSTALL_DIR!
        pause
        exit /b 1
    )
)

REM Copy executable
echo %INFO_PREFIX% Installing AMCheck C++ to !INSTALL_DIR!...
copy "%EXECUTABLE%" "!INSTALL_DIR!\amcheck.exe" >nul
if !errorLevel! neq 0 (
    echo %ERROR_PREFIX% Failed to copy executable
    pause
    exit /b 1
)

echo %SUCCESS_PREFIX% Executable copied to !INSTALL_DIR!\amcheck.exe

REM Add to PATH if not already there
echo %INFO_PREFIX% Checking PATH configuration...
echo !PATH! | findstr /i /c:"!INSTALL_DIR!" >nul
if !errorLevel! neq 0 (
    echo %WARNING_PREFIX% !INSTALL_DIR! is not in your PATH
    echo %INFO_PREFIX% Adding to PATH...
    
    REM Add to user PATH (works for both admin and non-admin)
    for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "USER_PATH=%%B"
    if "!USER_PATH!"=="" set "USER_PATH=!INSTALL_DIR!"
    if "!USER_PATH!"=="!INSTALL_DIR!" (
        REM PATH is just our directory
        set "NEW_PATH=!INSTALL_DIR!"
    ) else (
        REM Add our directory to existing PATH
        echo !USER_PATH! | findstr /i /c:"!INSTALL_DIR!" >nul
        if !errorLevel! neq 0 (
            set "NEW_PATH=!USER_PATH!;!INSTALL_DIR!"
        ) else (
            set "NEW_PATH=!USER_PATH!"
        )
    )
    
    reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "!NEW_PATH!" /f >nul
    if !errorLevel! == 0 (
        echo %SUCCESS_PREFIX% Added to user PATH
        echo %WARNING_PREFIX% Please restart Command Prompt or PowerShell for changes to take effect
    ) else (
        echo %WARNING_PREFIX% Failed to automatically add to PATH
        echo %WARNING_PREFIX% Please manually add !INSTALL_DIR! to your PATH environment variable
    )
) else (
    echo %SUCCESS_PREFIX% Directory already in PATH
)

REM Verify installation
echo %INFO_PREFIX% Verifying installation...
"!INSTALL_DIR!\amcheck.exe" --version >nul 2>&1
if !errorLevel! == 0 (
    echo %SUCCESS_PREFIX% Installation successful!
    echo.
    echo ==================================================================
    echo                     Usage Examples:
    echo ==================================================================
    echo   amcheck --help                    # Show help
    echo   amcheck --version                 # Show version  
    echo   amcheck POSCAR                    # Analyze crystal structure
    echo   amcheck -b BAND.dat               # Analyze band structure
    echo   amcheck -a POSCAR                 # Comprehensive search
    echo   amcheck --ahc POSCAR              # Anomalous Hall analysis
    echo ==================================================================
    echo.
    echo %SUCCESS_PREFIX% AMCheck C++ installation complete!
    echo %INFO_PREFIX% You may need to restart your command prompt for PATH changes
) else (
    echo %WARNING_PREFIX% Executable installed but may have dependency issues
    echo %INFO_PREFIX% Try running: "!INSTALL_DIR!\amcheck.exe" --help
)

goto :end

:uninstall
echo %INFO_PREFIX% Uninstalling AMCheck C++...

REM Determine installation directory based on admin status
net session >nul 2>&1
if %errorLevel% == 0 (
    set "INSTALL_DIR=C:\Program Files\AMCheck"
) else (
    set "INSTALL_DIR=%USERPROFILE%\AppData\Local\AMCheck"
)

if exist "!INSTALL_DIR!\amcheck.exe" (
    del "!INSTALL_DIR!\amcheck.exe" 2>nul
    rmdir "!INSTALL_DIR!" 2>nul
    echo %SUCCESS_PREFIX% AMCheck removed from !INSTALL_DIR!
    
    REM Remove from PATH
    for /f "tokens=2*" %%A in ('reg query "HKCU\Environment" /v PATH 2^>nul') do set "USER_PATH=%%B"
    if not "!USER_PATH!"=="" (
        set "NEW_PATH=!USER_PATH:;%INSTALL_DIR%=!"
        set "NEW_PATH=!NEW_PATH:%INSTALL_DIR%;=!"
        set "NEW_PATH=!NEW_PATH:%INSTALL_DIR%=!"
        reg add "HKCU\Environment" /v PATH /t REG_EXPAND_SZ /d "!NEW_PATH!" /f >nul
        echo %SUCCESS_PREFIX% Removed from PATH
    )
) else (
    echo %WARNING_PREFIX% AMCheck not found in !INSTALL_DIR!
)
goto :end

:show_help
echo AMCheck C++ Installation Script for Windows
echo.
echo Usage: %0 [OPTIONS]
echo.
echo OPTIONS:
echo   --help, -h        Show this help message  
echo   --uninstall, -u   Uninstall AMCheck from system
echo.
echo Installation Locations:
echo   Administrator:    C:\Program Files\AMCheck
echo   User:             %%USERPROFILE%%\AppData\Local\AMCheck
echo.
echo Examples:
echo   %0                # Install AMCheck
echo   %0 --uninstall    # Remove AMCheck
goto :end

:end
echo.
pause

# AMCheck C++ Installation Script for Windows (PowerShell)
# This script copies the executable to a directory in PATH for global access

param(
    [switch]$Uninstall,
    [switch]$Help
)

# Function to write colored output
function Write-Info($message) {
    Write-Host "[INFO] $message" -ForegroundColor Blue
}

function Write-Success($message) {
    Write-Host "[SUCCESS] $message" -ForegroundColor Green
}

function Write-Warning($message) {
    Write-Host "[WARNING] $message" -ForegroundColor Yellow
}

function Write-Error($message) {
    Write-Host "[ERROR] $message" -ForegroundColor Red
}

function Show-Help {
    Write-Host "AMCheck C++ Installation Script for Windows (PowerShell)" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Usage: .\install_windows.ps1 [OPTIONS]" -ForegroundColor White
    Write-Host ""
    Write-Host "OPTIONS:" -ForegroundColor White
    Write-Host "  -Help             Show this help message" -ForegroundColor Gray
    Write-Host "  -Uninstall        Uninstall AMCheck from system" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Installation Locations:" -ForegroundColor White
    Write-Host "  Administrator:    C:\Program Files\AMCheck" -ForegroundColor Gray
    Write-Host "  User:             $env:USERPROFILE\AppData\Local\AMCheck" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor White
    Write-Host "  .\install_windows.ps1             # Install AMCheck" -ForegroundColor Gray
    Write-Host "  .\install_windows.ps1 -Uninstall  # Remove AMCheck" -ForegroundColor Gray
}

function Find-Executable {
    $possiblePaths = @(
        "build\bin\amcheck.exe",
        "bin\amcheck.exe", 
        "amcheck.exe"
    )
    
    foreach ($path in $possiblePaths) {
        if (Test-Path $path) {
            return $path
        }
    }
    
    return $null
}

function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Add-ToPath {
    param($Directory)
    
    # Get current user PATH
    $userPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    
    # Check if directory is already in PATH
    if ($userPath -split ';' -contains $Directory) {
        Write-Success "Directory already in PATH"
        return
    }
    
    # Add to PATH
    if ([string]::IsNullOrEmpty($userPath)) {
        $newPath = $Directory
    } else {
        $newPath = "$userPath;$Directory"
    }
    
    try {
        [Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
        Write-Success "Added to user PATH"
        Write-Warning "Please restart PowerShell/Command Prompt for changes to take effect"
    } catch {
        Write-Warning "Failed to automatically add to PATH"
        Write-Warning "Please manually add $Directory to your PATH environment variable"
    }
}

function Remove-FromPath {
    param($Directory)
    
    $userPath = [Environment]::GetEnvironmentVariable("PATH", "User")
    if ([string]::IsNullOrEmpty($userPath)) {
        return
    }
    
    $pathArray = $userPath -split ';' | Where-Object { $_ -ne $Directory }
    $newPath = $pathArray -join ';'
    
    try {
        [Environment]::SetEnvironmentVariable("PATH", $newPath, "User")
        Write-Success "Removed from PATH"
    } catch {
        Write-Warning "Failed to remove from PATH"
    }
}

function Install-AMCheck {
    Write-Host "==================================================================" -ForegroundColor Cyan
    Write-Host "                 AMCheck C++ Installation Script for Windows" -ForegroundColor Cyan
    Write-Host "==================================================================" -ForegroundColor Cyan
    Write-Host ""
    
    # Find executable
    $executable = Find-Executable
    if (-not $executable) {
        Write-Error "AMCheck executable not found!"
        Write-Error "Please build the project first using: build_msys2.bat"
        Write-Error "Expected locations: build\bin\amcheck.exe, bin\amcheck.exe, or amcheck.exe"
        Read-Host "Press Enter to exit"
        exit 1
    }
    
    Write-Success "Found executable: $executable"
    
    # Get file size
    $fileSize = (Get-Item $executable).Length
    $fileSizeMB = [math]::Round($fileSize / 1MB, 2)
    Write-Info "Executable size: $fileSizeMB MB"
    
    # Determine installation directory
    if (Test-Administrator) {
        $installDir = "C:\Program Files\AMCheck"
        $installType = "system-wide (administrator)"
    } else {
        $installDir = "$env:USERPROFILE\AppData\Local\AMCheck"
        $installType = "user-local"
    }
    
    Write-Info "Installation type: $installType"
    Write-Info "Installation directory: $installDir"
    Write-Host ""
    
    # Ask for confirmation
    $confirm = Read-Host "Proceed with installation? [Y/n]"
    if ($confirm -eq 'n' -or $confirm -eq 'N') {
        Write-Info "Installation cancelled"
        Read-Host "Press Enter to exit"
        exit 0
    }
    
    # Create installation directory
    if (-not (Test-Path $installDir)) {
        Write-Info "Creating directory: $installDir"
        try {
            New-Item -ItemType Directory -Path $installDir -Force | Out-Null
        } catch {
            Write-Error "Failed to create directory: $installDir"
            Write-Error $_.Exception.Message
            Read-Host "Press Enter to exit"
            exit 1
        }
    }
    
    # Copy executable
    Write-Info "Installing AMCheck C++ to $installDir..."
    try {
        Copy-Item $executable "$installDir\amcheck.exe" -Force
        Write-Success "Executable copied to $installDir\amcheck.exe"
    } catch {
        Write-Error "Failed to copy executable"
        Write-Error $_.Exception.Message
        Read-Host "Press Enter to exit"
        exit 1
    }
    
    # Add to PATH
    Write-Info "Checking PATH configuration..."
    Add-ToPath $installDir
    
    # Verify installation
    Write-Info "Verifying installation..."
    try {
        $result = & "$installDir\amcheck.exe" --version 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Success "Installation successful!"
            Write-Host ""
            Write-Host "==================================================================" -ForegroundColor Green
            Write-Host "                     Usage Examples:" -ForegroundColor Green
            Write-Host "==================================================================" -ForegroundColor Green
            Write-Host "  amcheck --help                    # Show help" -ForegroundColor Gray
            Write-Host "  amcheck --version                 # Show version" -ForegroundColor Gray
            Write-Host "  amcheck POSCAR                    # Analyze crystal structure" -ForegroundColor Gray
            Write-Host "  amcheck -b BAND.dat               # Analyze band structure" -ForegroundColor Gray
            Write-Host "  amcheck -a POSCAR                 # Comprehensive search" -ForegroundColor Gray
            Write-Host "  amcheck --ahc POSCAR              # Anomalous Hall analysis" -ForegroundColor Gray
            Write-Host "==================================================================" -ForegroundColor Green
        } else {
            Write-Warning "Executable installed but may have dependency issues"
            Write-Info "Try running: $installDir\amcheck.exe --help"
        }
    } catch {
        Write-Warning "Could not verify installation"
        Write-Info "Try running: $installDir\amcheck.exe --help"
    }
    
    Write-Host ""
    Write-Success "AMCheck C++ installation complete!"
    Write-Info "You may need to restart PowerShell/Command Prompt for PATH changes"
}

function Uninstall-AMCheck {
    Write-Info "Uninstalling AMCheck C++..."
    
    # Determine installation directory based on admin status
    if (Test-Administrator) {
        $installDir = "C:\Program Files\AMCheck"
    } else {
        $installDir = "$env:USERPROFILE\AppData\Local\AMCheck"
    }
    
    if (Test-Path "$installDir\amcheck.exe") {
        try {
            Remove-Item "$installDir\amcheck.exe" -Force
            if (Test-Path $installDir) {
                Remove-Item $installDir -Force -Recurse
            }
            Write-Success "AMCheck removed from $installDir"
            
            # Remove from PATH
            Remove-FromPath $installDir
        } catch {
            Write-Error "Failed to remove AMCheck"
            Write-Error $_.Exception.Message
        }
    } else {
        Write-Warning "AMCheck not found in $installDir"
    }
}

# Main execution
if ($Help) {
    Show-Help
    exit 0
}

if ($Uninstall) {
    Uninstall-AMCheck
} else {
    Install-AMCheck
}

Write-Host ""
Read-Host "Press Enter to exit"

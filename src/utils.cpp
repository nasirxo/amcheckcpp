#include "amcheck.h"
#ifdef HAVE_CUDA
#include "cuda_accelerator.h"
#endif
#include <iostream>
#include <iomanip>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace amcheck {

// Function to detect if we should use Unicode or ASCII
bool should_use_unicode() {
#ifdef _WIN32
    // On Windows, check if we can set UTF-8 console mode
    UINT originalCP = GetConsoleOutputCP();
    bool canUseUnicode = SetConsoleOutputCP(CP_UTF8);
    if (canUseUnicode) {
        // Try to reset to original to be safe
        SetConsoleOutputCP(originalCP);
    }
    
    // For now, default to ASCII on Windows for maximum compatibility
    // Users can set AMCHECK_USE_UNICODE=1 environment variable to force Unicode
    const char* forceUnicode = std::getenv("AMCHECK_USE_UNICODE");
    return (forceUnicode && std::string(forceUnicode) == "1");
#else
    // Unix systems generally support Unicode well
    return true;
#endif
}

void print_banner() {
    std::cout << "\n";
    
    if (should_use_unicode()) {
        // Beautiful Unicode/ASCII art banner for Unicode-capable terminals
        std::cout << "╔══════════════════════════════════════════════════════════════════════════╗\n";
        std::cout << "║                                                                          ║\n";
        std::cout << "║         █████╗ ███╗   ███╗ ██████╗██╗  ██╗███████╗ ██████╗██╗  ██╗       ║\n";
        std::cout << "║        ██╔══██╗████╗ ████║██╔════╝██║  ██║██╔════╝██╔════╝██║ ██╔╝       ║\n";
        std::cout << "║        ███████║██╔████╔██║██║     ███████║█████╗  ██║     █████╔╝        ║\n";
        std::cout << "║        ██╔══██║██║╚██╔╝██║██║     ██╔══██║██╔══╝  ██║     ██╔═██╗        ║\n";
        std::cout << "║        ██║  ██║██║ ╚═╝ ██║╚██████╗██║  ██║███████╗╚██████╗██║  ██╗       ║\n";
        std::cout << "║        ╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝       ║\n";
        std::cout << "║                                                                          ║\n";
        std::cout << "║                         Altermagnet Detection Tool                       ║\n";
        std::cout << "║                        C++ High-Performance Edition                      ║\n";
        std::cout << "║                                                                          ║\n";
        std::cout << "║  ┌────────────────────────────────────────────────────────────────────┐  ║\n";
        std::cout << "║  │                             Authors                                │  ║\n";
        std::cout << "║  │                                                                    │  ║\n";
        std::cout << "║  │                     Nasir Ali  &  Shah Faisal                      │  ║\n";
        std::cout << "║  │                                                                    │  ║\n";
        std::cout << "║  │                       Department of Physics                        │  ║\n";
        std::cout << "║  │                 Quaid-i-Azam University, Islamabad                 │  ║\n";
        std::cout << "║  │                                                                    │  ║\n";
        std::cout << "║  │                  Supervisor: Prof. Dr. Gul Rahman                  │  ║\n";
        std::cout << "║  └────────────────────────────────────────────────────────────────────┘  ║\n";
        std::cout << "║                                                                          ║\n";
        std::cout << "║          Contact: nasiraliphy@gmail.com | shahf8885@gmail.com            ║\n";
        std::cout << "║                    Supervisor: gulrahman@qau.edu.pk                      ║\n";
        std::cout << "║              GitHub: https://github.com/nasirxo/amcheckcpp               ║\n";
        std::cout << "║                                                                          ║\n";
        std::cout << "║         © 2025 - All Rights Reserved | Licensed under BSD 3-Clause       ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════════════════╝\n";
    } else {
        // ASCII-only banner for Windows/terminals without Unicode support
        std::cout << "===============================================================================\n";
        std::cout << "                                                                               \n";
        std::cout << "              /\\   |\\    /|  /----  |   |  |----  /----  |   |                \n";
        std::cout << "             /  \\  | \\  / | |       |   |  |      |      |  /                 \n";
        std::cout << "            /____\\ |  \\/  | |       |___|  |----  |      |-<                  \n";
        std::cout << "           /      \\|      | |       |   |  |      |      |  \\                 \n";
        std::cout << "          /        |      |  \\____  |   |  |____   \\____ |   |                \n";
        std::cout << "                                                                               \n";
        std::cout << "                         Altermagnet Detection Tool                           \n";
        std::cout << "                        C++ High-Performance Edition                          \n";
        std::cout << "                                                                               \n";
        std::cout << "    +--------------------------------------------------------------------+    \n";
        std::cout << "    |                             Authors                                |    \n";
        std::cout << "    |                                                                    |    \n";
        std::cout << "    |                     Nasir Ali  &  Shah Faisal                      |    \n";
        std::cout << "    |                                                                    |    \n";
        std::cout << "    |                       Department of Physics                        |    \n";
        std::cout << "    |                 Quaid-i-Azam University, Islamabad                 |    \n";
        std::cout << "    |                                                                    |    \n";
        std::cout << "    |                  Supervisor: Prof. Dr. Gul Rahman                  |    \n";
        std::cout << "    +--------------------------------------------------------------------+    \n";
        std::cout << "                                                                               \n";
        std::cout << "          Contact: nasiraliphy@gmail.com | shahf8885@gmail.com                \n";
        std::cout << "                    Supervisor: gulrahman@qau.edu.pk                          \n";
        std::cout << "              GitHub: https://github.com/nasirxo/amcheckcpp                   \n";
        std::cout << "                                                                               \n";
        std::cout << "         (C) 2025 - All Rights Reserved | Licensed under BSD 3-Clause        \n";
        std::cout << "===============================================================================\n";
    }
    std::cout << "\n";
}

void print_version() {
    print_banner();
    std::cout << "AMCheck C++ v1.0.0 - Altermagnet Detection Suite\n";
    std::cout << "Built with: Eigen3, spglib, C++17";
#ifdef HAVE_CUDA
    std::cout << ", CUDA";
#endif
    std::cout << "\n";
    std::cout << "Features: POSCAR parsing, Symmetry analysis, Magnetic structure detection";
#ifdef HAVE_CUDA
    std::cout << ", GPU acceleration";
#endif
    std::cout << "\n";
    
    // Show GPU information if available
#ifdef HAVE_CUDA
    try {
        cuda::CudaSpinSearcher gpu_tester;
        if (gpu_tester.initialize()) {
            auto config = gpu_tester.get_config();
            std::cout << "GPU: " << config.device_name << " (CC " << config.compute_capability << ")\n";
        } else {
            std::cout << "GPU: Not available\n";
        }
    } catch (...) {
        std::cout << "GPU: Detection failed\n";
    }
#else
    std::cout << "GPU: Not compiled with CUDA support\n";
#endif
    
    std::cout << "\n";
}

// Function to test GPU availability
bool is_gpu_available() {
#ifdef HAVE_CUDA
    try {
        cuda::CudaSpinSearcher tester;
        return tester.initialize();
    } catch (...) {
        return false;
    }
#else
    return false;
#endif
}

void print_usage(const std::string& program_name) {
    std::cout << "\n";
    
    if (should_use_unicode()) {
        // Unicode version for capable terminals
        std::cout << "═══════════════════════════════════════════════════════════════════════\n";
        std::cout << "                              USAGE GUIDE\n";
        std::cout << "═══════════════════════════════════════════════════════════════════════\n";
        std::cout << "\n";
        std::cout << "Usage: " << program_name << " [OPTIONS] <structure_file>\n";
        std::cout << "   A powerful tool to detect altermagnetic materials using crystallographic analysis.\n";
        std::cout << "\n";
        std::cout << "OPTIONS:\n";
        std::cout << "   -h, --help         Show this help message\n";
        std::cout << "   -v, --verbose      Enable detailed output\n";
        std::cout << "   --version          Show version and credits\n";
        std::cout << "   -s, --symprec      Symmetry precision (default: " << DEFAULT_TOLERANCE << ")\n";
        std::cout << "   -t, --tolerance    Numerical tolerance (default: " << DEFAULT_TOLERANCE << ")\n";
        std::cout << "   -a, --search-all   Search all possible spin configurations (multithreaded)\n";
        std::cout << "   --ahc              Analyze Anomalous Hall Coefficient\n";
#ifdef HAVE_CUDA
        std::cout << "   --gpu              Enable GPU acceleration (default if available)\n";
        std::cout << "   --cpu, --no-gpu    Force CPU-only computation\n";
#endif
        std::cout << "\n";
        std::cout << "ARGUMENTS:\n";
        std::cout << "   structure_file     Crystal structure file (VASP POSCAR format)\n";
        std::cout << "\n";
        std::cout << "EXAMPLES:\n";
        std::cout << "   " << program_name << " POSCAR                    # Basic altermagnet check\n";
        std::cout << "   " << program_name << " -v --symprec 1e-5 POSCAR  # Verbose with custom precision\n";
        std::cout << "   " << program_name << " -a POSCAR                 # Search all spin configurations\n";
        std::cout << "   " << program_name << " --ahc POSCAR              # Anomalous Hall analysis\n";
#ifdef HAVE_CUDA
        std::cout << "   " << program_name << " -a --gpu POSCAR           # GPU-accelerated search\n";
#endif
        std::cout << "\n";
        std::cout << "💡 TIP: For best results, ensure your POSCAR file contains a well-converged structure!\n";
#ifdef HAVE_CUDA
        std::cout << "🚀 GPU acceleration available - use --gpu/--cpu to control!\n";
#endif
        std::cout << "\n";
    } else {
        // ASCII version for Windows/non-Unicode terminals
        std::cout << "=======================================================================\n";
        std::cout << "                              USAGE GUIDE\n";
        std::cout << "=======================================================================\n";
        std::cout << "\n";
        std::cout << "Usage: " << program_name << " [OPTIONS] <structure_file>\n";
        std::cout << "   A powerful tool to detect altermagnetic materials using crystallographic analysis.\n";
        std::cout << "\n";
        std::cout << "OPTIONS:\n";
        std::cout << "   -h, --help         Show this help message\n";
        std::cout << "   -v, --verbose      Enable detailed output\n";
        std::cout << "   --version          Show version and credits\n";
        std::cout << "   -s, --symprec      Symmetry precision (default: " << DEFAULT_TOLERANCE << ")\n";
        std::cout << "   -t, --tolerance    Numerical tolerance (default: " << DEFAULT_TOLERANCE << ")\n";
        std::cout << "   -a, --search-all   Search all possible spin configurations (multithreaded)\n";
        std::cout << "   --ahc              Analyze Anomalous Hall Coefficient\n";
#ifdef HAVE_CUDA
        std::cout << "   --gpu              Enable GPU acceleration (default if available)\n";
        std::cout << "   --cpu, --no-gpu    Force CPU-only computation\n";
#endif
        std::cout << "\n";
        std::cout << "ARGUMENTS:\n";
        std::cout << "   structure_file     Crystal structure file (VASP POSCAR format)\n";
        std::cout << "\n";
        std::cout << "EXAMPLES:\n";
        std::cout << "   " << program_name << " POSCAR                    # Basic altermagnet check\n";
        std::cout << "   " << program_name << " -v --symprec 1e-5 POSCAR  # Verbose with custom precision\n";
        std::cout << "   " << program_name << " -a POSCAR                 # Search all spin configurations\n";
        std::cout << "   " << program_name << " --ahc POSCAR              # Anomalous Hall analysis\n";
#ifdef HAVE_CUDA
        std::cout << "   " << program_name << " -a --gpu POSCAR           # GPU-accelerated search\n";
#endif
        std::cout << "\n";
        std::cout << "TIP: For best results, ensure your POSCAR file contains a well-converged structure!\n";
        std::cout << "     To enable Unicode output on Windows, set AMCHECK_USE_UNICODE=1\n";
#ifdef HAVE_CUDA
        std::cout << "     GPU acceleration available - use --gpu/--cpu to control!\n";
#endif
        std::cout << "\n";
    }
}

void print_spacegroup_info(const CrystalStructure& structure) {
#ifdef HAVE_SPGLIB
    std::string spacegroup = get_spacegroup_name(structure);
    std::cout << "Space Group: " << spacegroup << "\n";
#else
    std::cout << "Space Group: P1 (1) [spglib integration pending]\n";
#endif
}

void print_matrix(const Matrix3d& matrix, const std::string& name, int precision) {
    if (!name.empty()) {
        std::cout << name << ":\n";
    }
    std::cout << std::fixed << std::setprecision(precision);
    std::cout << "   [                                    ]\n";
    for (int i = 0; i < 3; ++i) {
        std::cout << "   [";
        for (int j = 0; j < 3; ++j) {
            std::cout << std::setw(precision + 5) << matrix(i, j);
            if (j < 2) std::cout << " ";
        }
        std::cout << "   ]\n";
    }
    std::cout << "   [                                    ]\n";
    std::cout << "\n";
}

void print_hall_vector(const Matrix3d& antisymmetric_tensor) {
    std::cout << "Hall Vector: [" 
              << antisymmetric_tensor(2, 1) << ", "
              << antisymmetric_tensor(0, 2) << ", "
              << antisymmetric_tensor(1, 0) << "]\n";
}

} // namespace amcheck

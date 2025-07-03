#include "amcheck.h"
#include <iostream>
#include <iomanip>

namespace amcheck {

void print_banner() {
    std::cout << "\n";
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
    std::cout << "\n";
}

void print_version() {
    print_banner();
    std::cout << "AMCheck C++ v1.0.0 - Altermagnet Detection Suite\n";
    std::cout << "Built with: Eigen3, spglib, C++17\n";
    std::cout << "Features: POSCAR parsing, Symmetry analysis, Magnetic structure detection\n";
    std::cout << "\n";
}

void print_usage(const std::string& program_name) {
    std::cout << "\n";
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
    std::cout << "\n";
    std::cout << "ARGUMENTS:\n";
    std::cout << "   structure_file     Crystal structure file (VASP POSCAR format)\n";
    std::cout << "\n";
    std::cout << "EXAMPLES:\n";
    std::cout << "   " << program_name << " POSCAR                    # Basic altermagnet check\n";
    std::cout << "   " << program_name << " -v --symprec 1e-5 POSCAR  # Verbose with custom precision\n";
    std::cout << "   " << program_name << " -a POSCAR                 # Search all spin configurations\n";
    std::cout << "   " << program_name << " --ahc POSCAR              # Anomalous Hall analysis\n";
    std::cout << "\n";
    std::cout << "TIP: For best results, ensure your POSCAR file contains a well-converged structure!\n";
    std::cout << "\n";
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

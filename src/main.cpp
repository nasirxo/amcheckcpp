#include "amcheck.h"
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// Forward declarations for functions in other files
namespace amcheck {
    void analyze_symmetry(CrystalStructure& structure, double tolerance = DEFAULT_TOLERANCE);
    void assign_spins_interactively(CrystalStructure& structure);
    void assign_spins_to_magnetic_atoms_only(CrystalStructure& structure);
    void assign_magnetic_moments_interactively(CrystalStructure& structure);
    void search_all_spin_configurations(const CrystalStructure& structure, double tolerance, bool verbose, bool use_gpu = true);
    void print_banner();
    void print_version();
    void print_usage(const std::string& program_name);
    void print_spacegroup_info(const CrystalStructure& structure);
    void print_matrix(const Matrix3d& matrix, const std::string& name, int precision);
    void print_hall_vector(const Matrix3d& antisymmetric_tensor);
}

using namespace amcheck;

struct Arguments {
    std::vector<std::string> files;
    bool verbose = false;
    bool show_help = false;
    bool show_version = false;
    bool ahc_mode = false;
    bool search_all_mode = false;
    bool use_gpu = true;  // Default to GPU if available
    bool force_cpu = false;
    double symprec = DEFAULT_TOLERANCE;
    double tolerance = DEFAULT_TOLERANCE;
};

Arguments parse_arguments(int argc, char* argv[]) {
    Arguments args;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            args.show_help = true;
        } else if (arg == "-v" || arg == "--verbose") {
            args.verbose = true;
        } else if (arg == "--version") {
            args.show_version = true;
        } else if (arg == "--ahc") {
            args.ahc_mode = true;
        } else if (arg == "-a" || arg == "--search-all") {
            args.search_all_mode = true;
        } else if (arg == "-s" || arg == "--symprec") {
            if (i + 1 < argc) {
                args.symprec = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--symprec requires a value");
            }
        } else if (arg == "-t" || arg == "--tolerance") {
            if (i + 1 < argc) {
                args.tolerance = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--tolerance requires a value");
            }
        } else if (arg == "--gpu") {
            args.use_gpu = true;
            args.force_cpu = false;
        } else if (arg == "--cpu" || arg == "--no-gpu") {
            args.use_gpu = false;
            args.force_cpu = true;
        } else if (arg[0] != '-') {
            args.files.push_back(arg);
        } else {
            throw std::invalid_argument("Unknown option: " + arg);
        }
    }
    
    return args;
}

void process_altermagnet_analysis(const std::string& filename, const Arguments& args) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                      ALTERMAGNET ANALYSIS\n";
    std::cout << "=======================================================================\n";
    std::cout << "Processing: " << filename << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    try {
        CrystalStructure structure;
        structure.read_from_file(filename);
        
        std::cout << "Structure loaded successfully!\n";
        
        // Analyze symmetry
        std::cout << "Analyzing crystal symmetry...\n";
        analyze_symmetry(structure, args.symprec);
        
        // Print space group information
        print_spacegroup_info(structure);
        
        if (args.verbose) {
            std::cout << "Number of symmetry operations: " 
                      << structure.symmetry_operations.size() << "\n";
        }
        
        // Create auxiliary file
        std::string aux_filename = filename + "_amcheck.vasp";
        std::cout << "\nWriting structure to auxiliary file: " 
                  << aux_filename << "\n";
        structure.write_vasp_file(aux_filename);
        
        // Get spins from user input (magnetic atoms only)
        std::cout << "\nSetting up magnetic configuration...\n";
        assign_spins_to_magnetic_atoms_only(structure);
        
        // Extract data for altermagnet analysis
        std::vector<Vector3d> positions = structure.get_all_scaled_positions();
        std::vector<std::string> chemical_symbols;
        std::vector<SpinType> spins;
        
        for (const auto& atom : structure.atoms) {
            chemical_symbols.push_back(atom.chemical_symbol);
            spins.push_back(atom.spin);
        }
        
        // Perform altermagnet analysis
        std::cout << "\nPerforming altermagnet detection...\n";
        bool is_am = is_altermagnet(
            structure.symmetry_operations,
            positions,
            structure.equivalent_atoms,
            chemical_symbols,
            spins,
            args.tolerance,
            args.verbose,
            false
        );
        
        std::cout << "\n";
        std::cout << "=======================================================================\n";
        if (is_am) {
            std::cout << "                         RESULT: ALTERMAGNET!\n";
            std::cout << "              Your material exhibits altermagnetic properties!\n";
        } else {
            std::cout << "                         RESULT: NOT ALTERMAGNET\n";
            std::cout << "             Your material does not show altermagnetic behavior.\n";
        }
        std::cout << "=======================================================================\n";
        std::cout << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}

void process_ahc_analysis(const std::string& filename, const Arguments& args) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                 ANOMALOUS HALL COEFFICIENT ANALYSIS\n";
    std::cout << "=======================================================================\n";
    std::cout << "Processing: " << filename << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    try {
        CrystalStructure structure;
        structure.read_from_file(filename);
        
        std::cout << "Structure loaded successfully!\n\n";
        std::cout << "List of atoms:\n";
        for (size_t i = 0; i < structure.atoms.size(); ++i) {
            Vector3d scaled_pos = structure.get_scaled_position(i);
            std::cout << "   " << structure.atoms[i].chemical_symbol << " " 
                      << scaled_pos.transpose() << "\n";
        }
        
        // Get magnetic moments from user
        std::cout << "\nSetting up magnetic moments...\n";
        assign_magnetic_moments_interactively(structure);
        
        // Get the regular space group information 
        std::cout << "\nCrystal Space Group Analysis:\n";
        print_spacegroup_info(structure);
        
        std::cout << "\nNote: Magnetic space group analysis requires manual interpretation\n";
        std::cout << "of the magnetic structure based on the assigned magnetic moments.\n";
        std::cout << "Current implementation uses simplified symmetry operations for AHC calculation.\n";
        
        // Generate rotations and time reversals (placeholder)
        std::vector<Matrix3d> rotations;
        std::vector<bool> time_reversals;
        
        // Add identity operation
        rotations.push_back(Matrix3d::Identity());
        time_reversals.push_back(false);
        
        // Add some example operations (in practice, these would come from magnetic symmetry analysis)
        Matrix3d inversion = -Matrix3d::Identity();
        rotations.push_back(inversion);
        time_reversals.push_back(true);
        
        if (args.verbose) {
            std::cout << "\nSymmetry operations:\n";
            for (size_t i = 0; i < rotations.size(); ++i) {
                std::cout << "   " << i + 1 << ": Time reversal: " << (time_reversals[i] ? "Yes" : "No") << "\n";
                print_matrix(rotations[i], "", 3);
            }
        }
        
        // Calculate symmetrized conductivity tensor
        Matrix3d S = symmetrized_conductivity_tensor(rotations, time_reversals);
        
        std::cout << "\n";
        print_matrix(S, "Conductivity Tensor", 7);
        if (args.verbose) {
            print_matrix_with_labels(S);
        }
        
        // Calculate antisymmetric part
        Matrix3d Sa = (S - S.transpose()) / 2.0;
        
        std::cout << "\n";
        print_matrix(Sa, "Antisymmetric Part (Anomalous Hall Effect)", 7);
        if (args.verbose) {
            print_matrix_with_labels(Sa);
        }
        
        std::cout << "\n";
        print_hall_vector(Sa);
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}

void process_search_all_analysis(const std::string& filename, const Arguments& args) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                    COMPREHENSIVE SPIN SEARCH MODE\n";
    std::cout << "=======================================================================\n";
    std::cout << "Processing: " << filename << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    try {
        CrystalStructure structure;
        structure.read_from_file(filename);
        
        std::cout << "Structure loaded successfully!\n";
        
        // Analyze symmetry
        std::cout << "Analyzing crystal symmetry...\n";
        analyze_symmetry(structure, args.symprec);
        
        // Print space group information
        print_spacegroup_info(structure);
        
        if (args.verbose) {
            std::cout << "Number of symmetry operations: " 
                      << structure.symmetry_operations.size() << "\n";
        }
        
        // Start comprehensive search
        search_all_spin_configurations(structure, args.tolerance, args.verbose, args.use_gpu && !args.force_cpu);
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}

int main(int argc, char* argv[]) {
    try {
        Arguments args = parse_arguments(argc, argv);
        
        if (args.show_help) {
            print_usage(argv[0]);
            return 0;
        }
        
        if (args.show_version) {
            print_version();
            return 0;
        }
        
        if (args.files.empty()) {
            print_banner();
            std::cerr << "Error: No input files specified\n\n";
            print_usage(argv[0]);
            return 1;
        }
        
        // Show banner for analysis mode
        if (!args.show_help && !args.show_version) {
            print_banner();
        }
        
        if (args.verbose) {
            std::cout << "Running in verbose mode\n";
        }
        
        for (const std::string& filename : args.files) {
            if (args.search_all_mode) {
                process_search_all_analysis(filename, args);
            } else if (args.ahc_mode) {
                process_ahc_analysis(filename, args);
            } else {
                process_altermagnet_analysis(filename, args);
            }
        }
        
        std::cout << "\n";
        std::cout << "=======================================================================\n";
        std::cout << "                            ANALYSIS COMPLETE\n";
        std::cout << "                    Thank you for using AMCheck C++!\n";
        std::cout << "\n";
        std::cout << "          Found this tool helpful? Please cite us in your research!\n";
        std::cout << "       Questions? Contact: nasiraliphy@gmail.com | shahf8885@gmail.com\n";
        std::cout << "=======================================================================\n";
        std::cout << "\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

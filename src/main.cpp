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
    void search_all_spin_configurations(const CrystalStructure& structure, const std::string& input_filename, double tolerance, bool verbose, bool use_gpu = true);
    void print_banner();
    void print_version();
    void print_usage(const std::string& program_name);
    void print_spacegroup_info(const CrystalStructure& structure);
    void print_matrix(const Matrix3d& matrix, const std::string& name, int precision);
    void print_hall_vector(const Matrix3d& antisymmetric_tensor);
    
    // Band analysis functions
    BandAnalysisResult analyze_band_file(const std::string& filename, double threshold, bool verbose);
    void print_band_analysis_summary(const BandAnalysisResult& result);
    void print_detailed_band_analysis(const BandAnalysisResult& result);
}

using namespace amcheck;

struct Arguments {
    std::vector<std::string> files;
    bool verbose = false;
    bool show_help = false;
    bool show_version = false;
    bool ahc_mode = false;
    bool search_all_mode = false;
    bool band_analysis_mode = false;
    bool use_gpu = true;  // Default to GPU if available
    bool force_cpu = false;
    double symprec = DEFAULT_TOLERANCE;
    double tolerance = DEFAULT_TOLERANCE;
    double band_threshold = 0.01;  // Default threshold for band analysis
    double xmin = 0.0;  // X-axis minimum for band plot
    double xmax = 0.0;  // X-axis maximum for band plot (0.0 means auto)
    double ymin = 0.0;  // Y-axis minimum for band plot
    double ymax = 0.0;  // Y-axis maximum for band plot (0.0 means auto)
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
        } else if (arg == "-b" || arg == "--band-analysis") {
            args.band_analysis_mode = true;
        } else if (arg == "--band-threshold") {
            if (i + 1 < argc) {
                args.band_threshold = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--band-threshold requires a value");
            }
        } else if (arg == "--xmin") {
            if (i + 1 < argc) {
                args.xmin = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--xmin requires a value");
            }
        } else if (arg == "--xmax") {
            if (i + 1 < argc) {
                args.xmax = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--xmax requires a value");
            }
        } else if (arg == "--ymin") {
            if (i + 1 < argc) {
                args.ymin = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--ymin requires a value");
            }
        } else if (arg == "--ymax") {
            if (i + 1 < argc) {
                args.ymax = std::stod(argv[++i]);
            } else {
                throw std::invalid_argument("--ymax requires a value");
            }
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
        search_all_spin_configurations(structure, filename, args.tolerance, args.verbose, args.use_gpu && !args.force_cpu);
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
}

void process_band_analysis(const std::string& filename, const Arguments& args) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                          BAND ANALYSIS MODE\n";
    std::cout << "=======================================================================\n";
    std::cout << "Processing: " << filename << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    try {
        BandAnalysisResult result = analyze_band_file(filename, args.band_threshold, args.verbose);
        
        // Print summary
        print_band_analysis_summary(result);
        
        // Print detailed analysis if verbose
        if (args.verbose) {
            print_detailed_band_analysis(result);
        }
        
        // Generate gnuplot script for visualizing bands with arrows
        
        // Check if we have a crystal structure file to extract symmetry information
        std::string poscar_filename = filename;
        size_t dot_pos = poscar_filename.find_last_of(".");
        if (dot_pos != std::string::npos) {
            poscar_filename = poscar_filename.substr(0, dot_pos) + ".poscar";
        }
        
        // Try to load crystal structure to determine k-points
        std::map<double, std::string> kpoint_labels;
        try {
            CrystalStructure structure;
            structure.read_from_file(poscar_filename);
            
#ifdef HAVE_SPGLIB
            // Get space group and lattice system using spglib
            analyze_symmetry_spglib(structure, args.tolerance);
            
            // Extract space group number from the name (e.g. "Pm-3m (221)")
            std::string spg_name = get_spacegroup_name(structure, args.tolerance);
            int spg_num = 0;
            size_t paren_pos = spg_name.find_last_of("(");
            if (paren_pos != std::string::npos) {
                std::string spg_num_str = spg_name.substr(paren_pos + 1);
                spg_num_str = spg_num_str.substr(0, spg_num_str.find_first_of(")"));
                try {
                    spg_num = std::stoi(spg_num_str);
                } catch (...) {
                    spg_num = 0;
                }
            }
            
            // Determine lattice system from space group number
            std::string lattice_system;
            if (spg_num >= 1 && spg_num <= 2) lattice_system = "triclinic";
            else if (spg_num >= 3 && spg_num <= 15) lattice_system = "monoclinic";
            else if (spg_num >= 16 && spg_num <= 74) lattice_system = "orthorhombic";
            else if (spg_num >= 75 && spg_num <= 142) lattice_system = "tetragonal";
            else if (spg_num >= 143 && spg_num <= 167) lattice_system = "trigonal";
            else if (spg_num >= 168 && spg_num <= 194) lattice_system = "hexagonal";
            else if (spg_num >= 195 && spg_num <= 230) lattice_system = "cubic";
            
            if (spg_num > 0 && !lattice_system.empty()) {
                std::cout << "\nDetected crystal information:\n";
                std::cout << "  Space group: " << spg_name << "\n";
                std::cout << "  Lattice system: " << lattice_system << "\n";
                
                // Generate k-points based on crystal symmetry
                kpoint_labels = get_high_symmetry_kpoints(spg_num, lattice_system);
                
                std::cout << "  Using high symmetry k-points for this structure.\n";
            }
#endif
        } catch (const std::exception& e) {
            std::cout << "\nCould not automatically determine high symmetry k-points: " << e.what() << "\n";
            std::cout << "Using default k-point labeling.\n";
        }
        
        generate_band_plot_script(result, filename, {args.xmin, args.xmax}, {args.ymin, args.ymax}, kpoint_labels);
        
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
            } else if (args.band_analysis_mode) {
                process_band_analysis(filename, args);
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

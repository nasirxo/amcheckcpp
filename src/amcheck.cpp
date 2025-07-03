#include "amcheck.h"
#ifdef HAVE_CUDA
#include "cuda_accelerator.h"
#endif
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <numeric>
#include <memory>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>

namespace amcheck {

Vector3d bring_in_cell(const Vector3d& r, double tol) {
    Vector3d result = r;
    
    for (int i = 0; i < 3; ++i) {
        result[i] = std::fmod(result[i], 1.0);
        if (result[i] < 0) result[i] += 1.0;
        
        // Handle numbers close to unity
        if (std::abs(1.0 - result[i]) < tol) {
            result[i] = 1.0 - result[i];
        }
    }
    
    return result;
}

bool check_altermagnetism_orbit(
    const std::vector<SymmetryOperation>& symops,
    const std::vector<Vector3d>& positions,
    const std::vector<SpinType>& spins,
    double tol,
    bool verbose,
    bool silent
) {
    // If orbit has multiplicity 1, it cannot be altermagnetic
    if (positions.size() == 1) return false;

    if (positions.size() != spins.size()) {
        throw std::invalid_argument("Number of positions must equal number of spins");
    }

    // For a given spin pattern, determine antisymmetry operations
    std::vector<bool> magn_symops_filter(symops.size(), true);
    
    for (size_t i = 0; i < positions.size(); ++i) {
        if (spins[i] != SpinType::UP && spins[i] != SpinType::DOWN) {
            continue;
        }

        for (size_t si = 0; si < symops.size(); ++si) {
            bool symop_is_present = false;
            const auto& [R, t] = symops[si];

            for (size_t j = 0; j < positions.size(); ++j) {
                if (!((spins[i] == SpinType::UP && spins[j] == SpinType::DOWN) ||
                      (spins[i] == SpinType::DOWN && spins[j] == SpinType::UP))) {
                    continue;
                }

                Vector3d dp = R * positions[i] + t - positions[j];
                dp = bring_in_cell(dp, tol);

                if (dp.norm() < tol) {
                    symop_is_present = true;
                    break;
                }
            }

            magn_symops_filter[si] = magn_symops_filter[si] && symop_is_present;
        }
    }

    std::vector<SymmetryOperation> magn_symops;
    for (size_t i = 0; i < symops.size(); ++i) {
        if (magn_symops_filter[i]) {
            magn_symops.push_back(symops[i]);
        }
    }

    if (magn_symops.empty()) {
        if (!silent && verbose) {
            std::cout << "Up and down sublattices are not symmetry-related: the material is Luttinger ferrimagnet!" << std::endl;
        }
        return false;
    }

    int N_magnetic_atoms = 2 * std::count(spins.begin(), spins.end(), SpinType::UP);

    std::vector<int> is_in_sym_related_pair(positions.size(), 0);
    std::vector<int> is_in_IT_related_pair(positions.size(), 0);

    // Check for inversion and translation relationships
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            if (!((spins[i] == SpinType::UP && spins[j] == SpinType::DOWN) ||
                  (spins[i] == SpinType::DOWN && spins[j] == SpinType::UP))) {
                continue;
            }

            Vector3d midpoint = (positions[i] + positions[j]) / 2.0;

            for (const auto& [R, t] : magn_symops) {
                Vector3d dp = R * positions[i] + t - positions[j];
                dp = bring_in_cell(dp, tol);

                if (dp.norm() < tol) {
                    is_in_sym_related_pair[i] = 1;
                    is_in_sym_related_pair[j] = 1;
                }

                // Check if symop is inversion
                if (std::abs(R.trace() + 3) < tol) {
                    Vector3d midpoint_prime = R * midpoint + t - midpoint;
                    midpoint_prime = bring_in_cell(midpoint_prime, tol);

                    if (midpoint_prime.norm() < tol) {
                        is_in_IT_related_pair[i] = 1;
                        is_in_IT_related_pair[j] = 1;
                        if (!silent && verbose) {
                            std::cout << "Atoms " << i+1 << " and " << j+1 
                                      << " are related by inversion (midpoint " 
                                      << midpoint.transpose() << ")." << std::endl;
                        }
                    }
                }

                // Check if symop is translation
                if (std::abs(R.trace() - 3) < tol && t.norm() > tol) {
                    Vector3d dp_trans = positions[i] + t - positions[j];
                    dp_trans = bring_in_cell(dp_trans, tol);

                    if (dp_trans.norm() < tol) {
                        is_in_IT_related_pair[i] = 1;
                        is_in_IT_related_pair[j] = 1;
                        if (!silent && verbose) {
                            std::cout << "Atoms " << i+1 << " and " << j+1 
                                      << " are related by translation " 
                                      << t.transpose() << "." << std::endl;
                        }
                    }
                }
            }
        }
    }

    if (!silent && verbose) {
        std::cout << "Atoms related by inversion/translation (1-yes, 0-no): ";
        for (int val : is_in_IT_related_pair) std::cout << val << " ";
        std::cout << std::endl;
        
        std::cout << "Atoms related by some symmetry (1-yes, 0-no): ";
        for (int val : is_in_sym_related_pair) std::cout << val << " ";
        std::cout << std::endl;
    }

    int sum_sym = std::accumulate(is_in_sym_related_pair.begin(), is_in_sym_related_pair.end(), 0);
    int sum_IT = std::accumulate(is_in_IT_related_pair.begin(), is_in_IT_related_pair.end(), 0);

    bool is_Luttinger_ferrimagnet = std::abs(sum_sym - N_magnetic_atoms) > tol;
    if (!silent && verbose && is_Luttinger_ferrimagnet) {
        std::cout << "Up and down sublattices are not related by symmetry: the material is Luttinger ferrimagnet!" << std::endl;
    }

    bool is_altermagnet = std::abs(sum_IT - N_magnetic_atoms) > tol;
    is_altermagnet = is_altermagnet && !is_Luttinger_ferrimagnet;

    return is_altermagnet;
}

bool is_altermagnet(
    const std::vector<SymmetryOperation>& symops,
    const std::vector<Vector3d>& atom_positions,
    const std::vector<int>& equiv_atoms,
    const std::vector<std::string>& chemical_symbols,
    const std::vector<SpinType>& spins,
    double tol,
    bool verbose,
    bool silent
) {
    bool altermagnet = false;
    bool check_was_performed = false;
    bool all_orbits_multiplicity_one = true;

    // Get unique orbit identifiers
    std::vector<int> unique_orbits = equiv_atoms;
    std::sort(unique_orbits.begin(), unique_orbits.end());
    unique_orbits.erase(std::unique(unique_orbits.begin(), unique_orbits.end()), unique_orbits.end());

    for (int u : unique_orbits) {
        std::vector<size_t> atom_ids;
        for (size_t i = 0; i < equiv_atoms.size(); ++i) {
            if (equiv_atoms[i] == u) {
                atom_ids.push_back(i);
            }
        }

        std::vector<Vector3d> orbit_positions;
        for (size_t id : atom_ids) {
            orbit_positions.push_back(atom_positions[id]);
        }

        if (!silent && verbose) {
            std::cout << "\nOrbit of " << chemical_symbols[atom_ids[0]] << " atoms:" << std::endl;
        }

        all_orbits_multiplicity_one = all_orbits_multiplicity_one && (orbit_positions.size() == 1);
        
        if (orbit_positions.size() == 1) {
            if (!silent) {
                std::cout << "Only one atom in the orbit: skipping." << std::endl;
            }
            continue;
        }

        std::vector<SpinType> orbit_spins;
        for (size_t id : atom_ids) {
            orbit_spins.push_back(spins[id]);
        }

        // Skip if orbit consists of non-magnetic atoms
        if (std::all_of(orbit_spins.begin(), orbit_spins.end(), 
                       [](SpinType s) { return s == SpinType::NONE; })) {
            if (!silent) {
                std::cout << "Group of non-magnetic atoms (" << chemical_symbols[u] << "): skipping." << std::endl;
            }
            continue;
        }

        // Check spin balance
        int N_u = std::count(orbit_spins.begin(), orbit_spins.end(), SpinType::UP);
        int N_d = std::count(orbit_spins.begin(), orbit_spins.end(), SpinType::DOWN);
        
        if (N_u != N_d) {
            throw std::invalid_argument("Number of up spins should equal number of down spins: got " +
                                      std::to_string(N_u) + " up and " + std::to_string(N_d) + " down spins!");
        }

        check_was_performed = true;
        bool is_orbit_altermagnetic = check_altermagnetism_orbit(symops, orbit_positions, orbit_spins, tol, verbose, silent);
        altermagnet = altermagnet || is_orbit_altermagnetic;
        
        if (!silent && verbose) {
            std::cout << "Altermagnetic orbit (" << chemical_symbols[u] << ")? " 
                      << (is_orbit_altermagnetic ? "true" : "false") << std::endl;
        }
    }

    if (!check_was_performed) {
        if (all_orbits_multiplicity_one) {
            altermagnet = false;
            if (!silent) {
                std::cout << "Note: in this structure, all orbits have multiplicity one.\n"
                          << "This material can only be a Luttinger ferrimagnet." << std::endl;
            }
        } else {
            throw std::runtime_error("Something is wrong with the description of magnetic atoms!\n"
                                   "Have you provided a non-magnetic/ferromagnetic material?");
        }
    }

    return altermagnet;
}

std::string spin_to_string(SpinType spin) {
    switch (spin) {
        case SpinType::UP: return "u";
        case SpinType::DOWN: return "d";
        case SpinType::NONE: return "n";
        default: return "n";
    }
}

SpinType string_to_spin(const std::string& s) {
    std::string lower = s;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "u") return SpinType::UP;
    if (lower == "d") return SpinType::DOWN;
    if (lower == "n") return SpinType::NONE;
    
    throw std::invalid_argument("Invalid spin designation: " + s);
}

std::vector<SpinType> input_spins(int num_atoms) {
    std::cout << "Type spin (u, U, d, D, n, N, nn or NN) for each of them (space separated):" << std::endl;
    
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    std::vector<std::string> spin_strings;
    std::string spin_str;
    
    while (iss >> spin_str) {
        spin_strings.push_back(spin_str);
    }

    // Empty line or "nn" marks all atoms as non-magnetic
    if (spin_strings.empty() || (spin_strings.size() == 1 && 
        (spin_strings[0] == "nn" || spin_strings[0] == "NN"))) {
        return std::vector<SpinType>(num_atoms, SpinType::NONE);
    }

    if (static_cast<int>(spin_strings.size()) != num_atoms) {
        throw std::invalid_argument("Wrong number of spins: got " + std::to_string(spin_strings.size()) +
                                  " instead of " + std::to_string(num_atoms));
    }

    std::vector<SpinType> spins;
    for (const auto& s : spin_strings) {
        spins.push_back(string_to_spin(s));
    }

    int N_u = std::count(spins.begin(), spins.end(), SpinType::UP);
    int N_d = std::count(spins.begin(), spins.end(), SpinType::DOWN);
    
    if (N_u != N_d) {
        throw std::invalid_argument("Number of up spins should equal number of down spins: got " +
                                  std::to_string(N_u) + " up and " + std::to_string(N_d) + " down spins!");
    }

    // If all atoms are non-magnetic
    if (N_u == 0) {
        return std::vector<SpinType>(num_atoms, SpinType::NONE);
    }

    return spins;
}

void print_matrix_with_labels(const Matrix3d& m, double tol) {
    std::vector<std::string> labels = {"xx", "yy", "zz", "yz", "xz", "xy", "zy", "zx", "yx"};
    std::vector<std::pair<int, int>> indices = {{0,0}, {1,1}, {2,2}, {1,2}, {0,2}, {0,1}, {2,1}, {2,0}, {1,0}};
    
    std::vector<std::vector<std::string>> symbolic_matrix(3, std::vector<std::string>(3, "0"));
    symbolic_matrix[0][0] = "xx";

    for (size_t i = 0; i < indices.size(); ++i) {
        auto [row, col] = indices[i];
        
        if (std::abs(m(row, col)) > tol) {
            bool found_match = false;
            
            for (size_t j = 0; j < i; ++j) {
                auto [prev_row, prev_col] = indices[j];
                
                if (std::abs(m(row, col) - m(prev_row, prev_col)) < tol) {
                    symbolic_matrix[row][col] = symbolic_matrix[prev_row][prev_col];
                    found_match = true;
                    break;
                } else if (std::abs(std::abs(m(row, col)) - std::abs(m(prev_row, prev_col))) < tol) {
                    symbolic_matrix[row][col] = "-" + symbolic_matrix[prev_row][prev_col];
                    found_match = true;
                    break;
                }
            }
            
            if (!found_match) {
                symbolic_matrix[row][col] = labels[i];
            }
        } else {
            symbolic_matrix[row][col] = "0";
        }
    }

    for (int i = 0; i < 3; ++i) {
        std::cout << "[";
        for (int j = 0; j < 3; ++j) {
            std::cout << std::setw(4) << symbolic_matrix[i][j];
            if (j < 2) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

Matrix3d symmetrized_conductivity_tensor(
    const std::vector<Matrix3d>& rotations,
    const std::vector<bool>& time_reversals
) {
    // Seed matrix for symmetrization
    Matrix3d seed;
    seed << 0.18848,  -0.52625,   0.047702,
            0.403317, -0.112371, -0.0564825,
           -0.352134,  0.350489,  0.0854533;
    
    Matrix3d seed_T = seed.transpose();
    Matrix3d S = Matrix3d::Zero();

    for (size_t i = 0; i < rotations.size(); ++i) {
        const Matrix3d& R = rotations[i];
        bool T = time_reversals[i];
        
        if (T) {
            S += R.inverse() * seed_T * R;
        } else {
            S += R.inverse() * seed * R;
        }
    }

    return S;
}

void search_all_spin_configurations(
    const CrystalStructure& structure,
    double tolerance,
    bool verbose,
    bool use_gpu
) {
    const size_t num_atoms = structure.atoms.size();
    
    // Get indices of magnetic atoms only
    std::vector<size_t> magnetic_indices = get_magnetic_atom_indices(structure);
    const size_t num_magnetic_atoms = magnetic_indices.size();
    
    // GPU acceleration setup
    bool use_cuda = false;
    std::string acceleration_method = "CPU";
    
#ifdef HAVE_CUDA
    std::unique_ptr<cuda::CudaSpinSearcher> cuda_searcher;
    
    if (use_gpu) {
        cuda_searcher = std::make_unique<cuda::CudaSpinSearcher>();
        if (cuda_searcher->initialize()) {
            use_cuda = true;
            acceleration_method = "GPU (CUDA)";
            
            auto config = cuda_searcher->get_config();
            std::cout << "\n🚀 GPU ACCELERATION ENABLED\n";
            std::cout << "Device: " << config.device_name << "\n";
            std::cout << "Compute Capability: " << config.compute_capability << "\n";
            std::cout << "Available VRAM: " << (config.memory_limit / (1024*1024)) << " MB\n";
        } else {
            std::cout << "\n⚠️  GPU requested but not available - falling back to CPU\n";
        }
    } else {
        std::cout << "\n💻 CPU-only mode selected\n";
    }
#else
    if (use_gpu) {
        std::cout << "\n⚠️  GPU requested but CUDA support not compiled - using CPU\n";
    }
#endif
    
    if (num_magnetic_atoms == 0) {
        std::cout << "\n=======================================================================\n";
        std::cout << "                  NO MAGNETIC ATOMS DETECTED\n";
        std::cout << "=======================================================================\n";
        std::cout << "Structure contains no potentially magnetic atoms.\n";
        std::cout << "Altermagnet analysis requires magnetic atoms.\n";
        std::cout << "=======================================================================\n\n";
        return;
    }
    
    // Calculate configurations based on magnetic atoms only (UP/DOWN, skip NONE)
    const size_t total_configurations = static_cast<size_t>(std::pow(2, num_magnetic_atoms));
    const unsigned int num_threads = std::thread::hardware_concurrency();
    
    // Generate output filename based on input structure
    std::string output_filename = "amcheck_spin_configurations.txt";
    
    if (num_magnetic_atoms > 20) {
        std::cout << "WARNING: Structure has " << num_magnetic_atoms << " magnetic atoms.\n";
        std::cout << "This will generate " << total_configurations << " configurations.\n";
        std::cout << "This may take a very long time. Continue? (y/N): ";
        std::string response;
        std::getline(std::cin, response);
        if (response != "y" && response != "Y") {
            std::cout << "Search cancelled.\n";
            return;
        }
    }
    
    std::cout << "\n=======================================================================\n";
    std::cout << "                  MULTITHREADED SPIN CONFIGURATION SEARCH\n";
    std::cout << "                           (MAGNETIC ATOMS ONLY)\n";
    std::cout << "=======================================================================\n";
    std::cout << "Structure: " << num_atoms << " total atoms (" << num_magnetic_atoms << " magnetic)\n";
    std::cout << "Total configurations to test: " << total_configurations << "\n";
    std::cout << "Acceleration method: " << acceleration_method << "\n";
    std::cout << "CPU cores available: " << num_threads << "\n";
    std::cout << "Tolerance: " << tolerance << "\n";
    std::cout << "Output file: " << output_filename << "\n";
    std::cout << "=======================================================================\n\n";
    
    // Print atomic structure information
    std::cout << "Magnetic atoms to be configured:\n";
    std::cout << "-----------------------------------------------------------------------\n";
    for (size_t i = 0; i < magnetic_indices.size(); ++i) {
        size_t atom_idx = magnetic_indices[i];
        Vector3d pos = structure.get_scaled_position(atom_idx);
        std::cout << "Mag " << std::setw(2) << (i + 1) << " (Atom " << std::setw(2) << (atom_idx + 1) << "): " 
                  << std::setw(2) << structure.atoms[atom_idx].chemical_symbol 
                  << " at (" << std::fixed << std::setprecision(6)
                  << std::setw(9) << pos[0] << ", " 
                  << std::setw(9) << pos[1] << ", " 
                  << std::setw(9) << pos[2] << ")\n";
    }
    std::cout << "-----------------------------------------------------------------------\n\n";
    
    std::vector<SpinConfiguration> altermagnetic_configs;
    std::mutex results_mutex;
    std::mutex output_mutex;  // For thread-safe console output
    std::atomic<size_t> completed_configs(0);
    std::atomic<size_t> altermagnetic_count(0);
    
    // GPU-accelerated search if available
#ifdef HAVE_CUDA
    if (use_cuda && cuda_searcher) {
        std::cout << "Starting GPU-accelerated search...\n";
        
        try {
            std::vector<SpinConfiguration> gpu_results = cuda_searcher->search_configurations(
                structure, magnetic_indices, tolerance, verbose
            );
            
            // Process GPU results
            for (const auto& config : gpu_results) {
                if (config.is_altermagnetic) {
                    altermagnetic_configs.push_back(config);
                    altermagnetic_count++;
                    
                    // Display configuration immediately when found
                    std::cout << "GPU FOUND Config #" << std::setw(8) << config.configuration_id << ": ";
                    
                    // Show compact spin pattern
                    for (size_t j = 0; j < config.spins.size(); ++j) {
                        if (j > 0) std::cout << " ";
                        std::cout << spin_to_string(config.spins[j]);
                    }
                    
                    // Show detailed atomic assignment
                    std::cout << " [";
                    for (size_t j = 0; j < structure.atoms.size(); ++j) {
                        if (j > 0) std::cout << " ";
                        std::cout << structure.atoms[j].chemical_symbol << (j+1) << ":" << spin_to_string(config.spins[j]);
                    }
                    std::cout << "]\n" << std::flush;
                }
            }
            
            completed_configs = total_configurations;
            
            std::cout << "\r" << std::string(80, ' ') << "\r";  // Clear progress line
            std::cout << "GPU search completed: " << total_configurations << "/" << total_configurations 
                      << " configurations processed\n\n";
            
        } catch (const std::exception& e) {
            std::cout << "GPU search failed: " << e.what() << "\n";
            std::cout << "Falling back to CPU search...\n";
            use_cuda = false;
            acceleration_method = "CPU (GPU fallback)";
        }
    }
    
    // Only run CPU search if GPU didn't complete the task
    if (!use_cuda) {
#endif
    
        // CPU multithreaded search (fallback or primary method)
        
        // Create worker function - only considers magnetic atoms
        auto worker = [&](size_t start_config, size_t end_config) {
            std::vector<SpinConfiguration> local_results;
        
        for (size_t config_id = start_config; config_id < end_config; ++config_id) {
            // Initialize all spins to NONE first
            std::vector<SpinType> spins(num_atoms, SpinType::NONE);
            
            // Generate spin configuration for magnetic atoms only (UP=0, DOWN=1)
            size_t temp_id = config_id;
            
            for (size_t i = 0; i < num_magnetic_atoms; ++i) {
                size_t atom_idx = magnetic_indices[i];
                int spin_val = temp_id % 2;  // Only UP (0) or DOWN (1)
                temp_id /= 2;
                
                spins[atom_idx] = (spin_val == 0) ? SpinType::UP : SpinType::DOWN;
            }
            
            // Extract data for altermagnet analysis
            std::vector<Vector3d> positions = structure.get_all_scaled_positions();
            std::vector<std::string> chemical_symbols;
            
            for (const auto& atom : structure.atoms) {
                chemical_symbols.push_back(atom.chemical_symbol);
            }
            
            // Check if this configuration is altermagnetic with error handling
            bool is_am = false;
            try {
                is_am = is_altermagnet(
                    structure.symmetry_operations,
                    positions,
                    structure.equivalent_atoms,
                    chemical_symbols,
                    spins,
                    tolerance,
                    false,  // not verbose
                    true    // silent
                );
            } catch (const std::exception& e) {
                // Skip configurations that violate constraints (e.g., unequal up/down spins per orbit)
                completed_configs++;
                continue;
            }
            
            if (is_am) {
                SpinConfiguration config;
                config.spins = spins;
                config.is_altermagnetic = true;
                config.configuration_id = config_id;
                local_results.push_back(config);
                altermagnetic_count++;
                
                // Display configuration immediately when found
                {
                    std::lock_guard<std::mutex> lock(output_mutex);
                    std::cout << "\r" << std::string(80, ' ') << "\r";  // Clear progress line
                    std::cout << "FOUND Config #" << std::setw(8) << config_id << ": ";
                    
                    // Show compact spin pattern
                    for (size_t j = 0; j < spins.size(); ++j) {
                        if (j > 0) std::cout << " ";
                        std::cout << spin_to_string(spins[j]);
                    }
                    
                    // Show detailed atomic assignment
                    std::cout << " [";
                    for (size_t j = 0; j < structure.atoms.size(); ++j) {
                        if (j > 0) std::cout << " ";
                        std::cout << structure.atoms[j].chemical_symbol << (j+1) << ":" << spin_to_string(spins[j]);
                    }
                    std::cout << "]\n" << std::flush;
                }
            }
            
            completed_configs++;
            
            // Progress reporting (every 100000 configurations or 1% whichever is smaller)
            size_t progress_interval = std::min(static_cast<size_t>(100000), 
                                               std::max(static_cast<size_t>(1), total_configurations / 100));
            if (completed_configs % progress_interval == 0) {
                std::lock_guard<std::mutex> lock(output_mutex);
                double progress = 100.0 * completed_configs / total_configurations;
                std::cout << "\rProgress: " << std::fixed << std::setprecision(1) 
                          << progress << "% (" << completed_configs << "/" 
                          << total_configurations << ") - Found: " 
                          << altermagnetic_count << " altermagnetic configs" << std::flush;
            }
        }
        
        // Merge local results into global results
        std::lock_guard<std::mutex> lock(results_mutex);
        altermagnetic_configs.insert(altermagnetic_configs.end(), 
                                   local_results.begin(), local_results.end());
    };
    
    // Launch threads
    std::vector<std::thread> threads;
    const size_t configs_per_thread = total_configurations / num_threads;
    const size_t remaining_configs = total_configurations % num_threads;
    
    for (unsigned int t = 0; t < num_threads; ++t) {
        size_t start_config = t * configs_per_thread;
        size_t end_config = (t + 1) * configs_per_thread;
        
        // Distribute remaining configurations among first threads
        if (t < remaining_configs) {
            start_config += t;
            end_config += t + 1;
        } else {
            start_config += remaining_configs;
            end_config += remaining_configs;
        }
        
        threads.emplace_back(worker, start_config, end_config);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "\rProgress: 100.0% (" << total_configurations << "/" 
              << total_configurations << ") - Found: " 
              << altermagnetic_count << " altermagnetic configs\n\n";
    
#ifdef HAVE_CUDA
    } // End of CPU search conditional block
#endif
    
    // Display results
    std::cout << "=======================================================================\n";
    std::cout << "                           SEARCH RESULTS\n";
    std::cout << "=======================================================================\n";
    std::cout << "Total configurations tested: " << total_configurations << "\n";
    std::cout << "Altermagnetic configurations found: " << altermagnetic_configs.size() << "\n";
    
    if (altermagnetic_configs.empty()) {
        std::cout << "\nNo altermagnetic configurations found for this structure.\n";
        std::cout << "=======================================================================\n";
        return;
    }
    
    // Sort by configuration ID for consistent output
    std::sort(altermagnetic_configs.begin(), altermagnetic_configs.end(),
              [](const SpinConfiguration& a, const SpinConfiguration& b) {
                  return a.configuration_id < b.configuration_id;
              });
    
    // Save all configurations to file
    std::ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        std::cerr << "ERROR: Could not open output file: " << output_filename << "\n";
        return;
    }
    
    // Write header to file
    outfile << "# AMCheck C++ - Altermagnetic Spin Configurations\n";
    outfile << "# Generated on: " << __DATE__ << " " << __TIME__ << "\n";
    outfile << "# Structure: " << num_atoms << " atoms\n";
    outfile << "# Acceleration method: " << acceleration_method << "\n";
    outfile << "# Total configurations tested: " << total_configurations << "\n";
    outfile << "# Altermagnetic configurations found: " << altermagnetic_configs.size() << "\n";
    outfile << "# Tolerance: " << tolerance << "\n";
    outfile << "#\n";
    outfile << "# Atomic structure:\n";
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        Vector3d pos = structure.get_scaled_position(i);
        outfile << "# Atom " << std::setw(2) << (i + 1) << ": " 
                << std::setw(2) << structure.atoms[i].chemical_symbol 
                << " at (" << std::fixed << std::setprecision(6)
                << std::setw(9) << pos[0] << ", " 
                << std::setw(9) << pos[1] << ", " 
                << std::setw(9) << pos[2] << ")\n";
    }
    outfile << "#\n";
    outfile << "# Format: ConfigID | Spin_Pattern | Detailed_Assignment\n";
    outfile << "#         u = up, d = down, n = none\n";
    outfile << "#\n\n";
    
    // Write all configurations to file
    for (const auto& config : altermagnetic_configs) {
        // Configuration ID and compact spin pattern
        outfile << std::setw(8) << config.configuration_id << " | ";
        
        for (size_t j = 0; j < config.spins.size(); ++j) {
            if (j > 0) outfile << " ";
            outfile << spin_to_string(config.spins[j]);
        }
        outfile << " | ";
        
        // Detailed atomic assignment
        for (size_t j = 0; j < structure.atoms.size(); ++j) {
            if (j > 0) outfile << " ";
            outfile << structure.atoms[j].chemical_symbol << j+1 << ":" << spin_to_string(config.spins[j]);
        }
        outfile << "\n";
    }
    
    outfile.close();
    std::cout << "\nAll " << altermagnetic_configs.size() 
              << " altermagnetic configurations saved to: " << output_filename << "\n";
    
    std::cout << "\nFirst 50 altermagnetic configurations:\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    for (size_t i = 0; i < std::min(static_cast<size_t>(50), altermagnetic_configs.size()); ++i) {
        const auto& config = altermagnetic_configs[i];
        std::cout << "Config #" << std::setw(8) << config.configuration_id << ": ";
        
        // Show compact spin pattern
        for (size_t j = 0; j < config.spins.size(); ++j) {
            if (j > 0) std::cout << " ";
            std::cout << spin_to_string(config.spins[j]);
        }
        
        // Show detailed atomic assignment
        std::cout << " [";
        for (size_t j = 0; j < structure.atoms.size(); ++j) {
            if (j > 0) std::cout << " ";
            std::cout << structure.atoms[j].chemical_symbol << (j+1) << ":" << spin_to_string(config.spins[j]);
        }
        std::cout << "]\n";
    }
    
    if (altermagnetic_configs.size() > 50) {
        std::cout << "... and " << (altermagnetic_configs.size() - 50) 
                  << " more configurations (see " << output_filename << " for complete list).\n";
    }
        
        if (verbose && altermagnetic_configs.size() <= 10) {
            std::cout << "\nDetailed analysis of altermagnetic configurations:\n";
            std::cout << "-----------------------------------------------------------------------\n";
            
            for (const auto& config : altermagnetic_configs) {
                std::cout << "\nConfiguration #" << config.configuration_id << ":\n";
                
                // Show atomic details with positions
                for (size_t j = 0; j < structure.atoms.size(); ++j) {
                    Vector3d pos = structure.get_scaled_position(j);
                    std::cout << "  Atom " << std::setw(2) << (j+1) << ": " 
                              << std::setw(2) << structure.atoms[j].chemical_symbol 
                              << " at (" << std::fixed << std::setprecision(6)
                              << std::setw(9) << pos[0] << ", " 
                              << std::setw(9) << pos[1] << ", " 
                              << std::setw(9) << pos[2] << ") "
                              << "[" << spin_to_string(config.spins[j]) << "]\n";
                }
            }
        }
    
    std::cout << "\nSummary:\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "- Acceleration method: " << acceleration_method << "\n";
    std::cout << "- Total configurations tested: " << total_configurations << "\n";
    std::cout << "- Altermagnetic configurations found: " << altermagnetic_configs.size() << "\n";
    std::cout << "- Success rate: " << std::fixed << std::setprecision(2) 
              << (100.0 * altermagnetic_configs.size() / total_configurations) << "%\n";
    std::cout << "- Results saved to: " << output_filename << "\n";
    
    std::cout << "=======================================================================\n";
}

} // namespace amcheck

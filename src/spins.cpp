#include "amcheck.h"
#include <iostream>
#include <stdexcept>
#include <set>
#include <unordered_set>

namespace amcheck {

// Function to analyze symmetry - declaration for use in this file
void analyze_symmetry(CrystalStructure& structure, double tolerance = DEFAULT_TOLERANCE);

// Magnetic elements database - transition metals and lanthanides/actinides
bool is_magnetic_element(const std::string& chemical_symbol) {
    static const std::unordered_set<std::string> magnetic_elements = {
        // 3d transition metals
        "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
        // 4d transition metals  
        "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd",
        // 5d transition metals
        "La", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",
        // 6d transition metals
        "Ac", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds", "Rg", "Cn",
        // Lanthanides (4f)
        "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu",
        // Actinides (5f)
        "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",
        // Some p-block elements that can be magnetic in certain conditions
        "B", "C", "N", "O", "F", "S", "Cl"
    };
    
    return magnetic_elements.find(chemical_symbol) != magnetic_elements.end();
}

// Get indices of atoms that can be magnetic
std::vector<size_t> get_magnetic_atom_indices(const CrystalStructure& structure) {
    std::vector<size_t> magnetic_indices;
    
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        if (is_magnetic_element(structure.atoms[i].chemical_symbol)) {
            magnetic_indices.push_back(i);
        }
    }
    
    return magnetic_indices;
}

// Get orbit indices that contain magnetic atoms
std::vector<size_t> get_magnetic_orbit_indices(const CrystalStructure& structure) {
    std::vector<size_t> magnetic_atom_indices = get_magnetic_atom_indices(structure);
    std::set<int> magnetic_orbit_ids;
    
    for (size_t atom_idx : magnetic_atom_indices) {
        magnetic_orbit_ids.insert(structure.equivalent_atoms[atom_idx]);
    }
    
    std::vector<size_t> magnetic_orbit_indices;
    for (int orbit_id : magnetic_orbit_ids) {
        magnetic_orbit_indices.push_back(static_cast<size_t>(orbit_id));
    }
    
    return magnetic_orbit_indices;
}

void assign_spins_to_magnetic_atoms_only(CrystalStructure& structure) {
    std::cout << "Auto-detecting magnetic atoms and assigning spins..." << std::endl;
    
    // First, set all atoms to non-magnetic
    for (auto& atom : structure.atoms) {
        atom.spin = SpinType::NONE;
    }
    
    // Get magnetic atom indices
    std::vector<size_t> magnetic_indices = get_magnetic_atom_indices(structure);
    
    if (magnetic_indices.empty()) {
        std::cout << "No magnetic atoms detected in the structure.\n";
        std::cout << "All atoms are set as non-magnetic (SpinType::NONE).\n";
        return;
    }
    
    std::cout << "Detected " << magnetic_indices.size() << " potentially magnetic atoms:\n";
    for (size_t idx : magnetic_indices) {
        Vector3d pos = structure.get_scaled_position(idx);
        std::cout << "  Atom " << (idx + 1) << ": " << structure.atoms[idx].chemical_symbol 
                  << " at (" << pos.transpose() << ")\n";
    }
    std::cout << "\n";
    
    // Group magnetic atoms by orbits
    std::map<int, std::vector<size_t>> magnetic_orbits;
    for (size_t idx : magnetic_indices) {
        int orbit_id = structure.equivalent_atoms[idx];
        magnetic_orbits[orbit_id].push_back(idx);
    }
    
    std::cout << "Assigning spins to magnetic atom orbits..." << std::endl;
    
    for (const auto& [orbit_id, atom_indices] : magnetic_orbits) {
        if (atom_indices.empty()) continue;
        
        std::cout << "\nMagnetic orbit of " << structure.atoms[atom_indices[0]].chemical_symbol 
                  << " atoms at positions:" << std::endl;
        
        for (size_t i = 0; i < atom_indices.size(); ++i) {
            size_t atom_idx = atom_indices[i];
            Vector3d scaled_pos = structure.get_scaled_position(atom_idx);
            std::cout << "  " << atom_idx + 1 << " (" << i + 1 << ") " 
                      << scaled_pos.transpose() << std::endl;
        }
        
        if (atom_indices.size() == 1) {
            std::cout << "Only one atom in the orbit: setting as UP spin." << std::endl;
            structure.atoms[atom_indices[0]].spin = SpinType::UP;
            continue;
        }
        
        try {
            std::vector<SpinType> orbit_spins = input_spins(static_cast<int>(atom_indices.size()));
            
            for (size_t i = 0; i < atom_indices.size(); ++i) {
                structure.atoms[atom_indices[i]].spin = orbit_spins[i];
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Setting all atoms in this orbit as non-magnetic." << std::endl;
            for (size_t atom_idx : atom_indices) {
                structure.atoms[atom_idx].spin = SpinType::NONE;
            }
        }
    }
    
    // Summary
    int total_up = 0, total_down = 0, total_none = 0, total_magnetic = 0;
    for (const auto& atom : structure.atoms) {
        switch (atom.spin) {
            case SpinType::UP: total_up++; total_magnetic++; break;
            case SpinType::DOWN: total_down++; total_magnetic++; break;
            case SpinType::NONE: total_none++; break;
        }
    }
    
    std::cout << "\n=== SPIN ASSIGNMENT SUMMARY ===\n";
    std::cout << "Total atoms: " << structure.atoms.size() << "\n";
    std::cout << "Magnetic atoms: " << total_magnetic << " (UP: " << total_up << ", DOWN: " << total_down << ")\n";
    std::cout << "Non-magnetic atoms: " << total_none << "\n";
    std::cout << "=====================================\n\n";
}

void assign_spins_interactively(CrystalStructure& structure) {
    std::cout << "Assigning spins to atomic orbits..." << std::endl;
    
    // Get unique orbit identifiers
    std::vector<int> unique_orbits = structure.equivalent_atoms;
    std::sort(unique_orbits.begin(), unique_orbits.end());
    unique_orbits.erase(std::unique(unique_orbits.begin(), unique_orbits.end()), unique_orbits.end());
    
    for (int orbit_id : unique_orbits) {
        std::vector<size_t> atom_indices;
        for (size_t i = 0; i < structure.equivalent_atoms.size(); ++i) {
            if (structure.equivalent_atoms[i] == orbit_id) {
                atom_indices.push_back(i);
            }
        }
        
        if (atom_indices.empty()) continue;
        
        std::cout << "\nOrbit of " << structure.atoms[atom_indices[0]].chemical_symbol 
                  << " atoms at positions:" << std::endl;
        
        for (size_t i = 0; i < atom_indices.size(); ++i) {
            size_t atom_idx = atom_indices[i];
            Vector3d scaled_pos = structure.get_scaled_position(atom_idx);
            std::cout << atom_idx + 1 << " (" << i + 1 << ") " 
                      << scaled_pos.transpose() << std::endl;
        }
        
        if (atom_indices.size() == 1) {
            std::cout << "Only one atom in the orbit: skipping." << std::endl;
            structure.atoms[atom_indices[0]].spin = SpinType::NONE;
            continue;
        }
        
        try {
            std::vector<SpinType> orbit_spins = input_spins(static_cast<int>(atom_indices.size()));
            
            for (size_t i = 0; i < atom_indices.size(); ++i) {
                structure.atoms[atom_indices[i]].spin = orbit_spins[i];
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            std::cout << "Setting all atoms in this orbit as non-magnetic." << std::endl;
            for (size_t atom_idx : atom_indices) {
                structure.atoms[atom_idx].spin = SpinType::NONE;
            }
        }
    }
}

void assign_magnetic_moments_interactively(CrystalStructure& structure) {
    std::cout << "Assigning magnetic moments to atoms..." << std::endl;
    std::cout << "List of atoms:" << std::endl;
    
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        Vector3d scaled_pos = structure.get_scaled_position(i);
        std::cout << structure.atoms[i].chemical_symbol << " " 
                  << scaled_pos.transpose() << std::endl;
    }
    
    std::cout << "\nType magnetic moments for each atom in Cartesian coordinates" << std::endl;
    std::cout << "('mx my mz' or empty line for non-magnetic atom):" << std::endl;
    
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        std::cout << "Atom " << i + 1 << " (" << structure.atoms[i].chemical_symbol << "): ";
        
        std::string line;
        std::getline(std::cin, line);
        
        if (line.empty()) {
            structure.atoms[i].magnetic_moment = Vector3d::Zero();
            continue;
        }
        
        std::istringstream iss(line);
        std::vector<double> moments;
        double moment;
        
        while (iss >> moment) {
            moments.push_back(moment);
        }
        
        if (moments.size() != 3) {
            throw std::invalid_argument("Three numbers for magnetic moment definition were expected!");
        }
        
        structure.atoms[i].magnetic_moment = Vector3d(moments[0], moments[1], moments[2]);
    }
    
    std::cout << "Assigned magnetic moments:" << std::endl;
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        std::cout << structure.atoms[i].magnetic_moment.transpose() << std::endl;
    }
}

} // namespace amcheck

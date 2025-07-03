#include "amcheck.h"
#include <iostream>
#include <stdexcept>

namespace amcheck {

// Function to analyze symmetry - declaration for use in this file
void analyze_symmetry(CrystalStructure& structure, double tolerance = DEFAULT_TOLERANCE);

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

#include "amcheck.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

#ifdef HAVE_SPGLIB
#include <spglib.h>
#endif

namespace amcheck {

#ifdef HAVE_SPGLIB
std::string get_spacegroup_name(const CrystalStructure& structure, double symprec) {
    // Prepare data for spglib
    double lattice[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            lattice[i][j] = structure.cell(i, j);
        }
    }
    
    std::vector<double> positions_flat;
    std::vector<int> types;
    
    for (const auto& atom : structure.atoms) {
        positions_flat.push_back(atom.position[0]);
        positions_flat.push_back(atom.position[1]);
        positions_flat.push_back(atom.position[2]);
        types.push_back(atom.atomic_number);
    }
    
    // Reshape positions for spglib (num_atoms x 3)
    double (*positions)[3] = reinterpret_cast<double(*)[3]>(positions_flat.data());
    
    char symbol[11];
    int spacegroup_number = spg_get_international(symbol, lattice, positions, types.data(), 
                                                 static_cast<int>(structure.atoms.size()), symprec);
    
    if (spacegroup_number == 0) {
        return "Unknown";
    }
    
    return std::string(symbol) + " (" + std::to_string(spacegroup_number) + ")";
}

std::vector<SymmetryOperation> get_symmetry_operations(const CrystalStructure& structure, double symprec) {
    std::vector<SymmetryOperation> operations;
    
    // Prepare data for spglib
    double lattice[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            lattice[i][j] = structure.cell(i, j);
        }
    }
    
    std::vector<double> positions_flat;
    std::vector<int> types;
    
    for (const auto& atom : structure.atoms) {
        positions_flat.push_back(atom.position[0]);
        positions_flat.push_back(atom.position[1]);
        positions_flat.push_back(atom.position[2]);
        types.push_back(atom.atomic_number);
    }
    
    double (*positions)[3] = reinterpret_cast<double(*)[3]>(positions_flat.data());
    
    // Get symmetry operations
    int max_size = 192; // Maximum number of operations in any space group
    int rotation[max_size][3][3];
    double translation[max_size][3];
    
    int num_operations = spg_get_symmetry(rotation, translation, max_size, lattice, positions, 
                                         types.data(), static_cast<int>(structure.atoms.size()), symprec);
    
    if (num_operations == 0) {
        // Fallback to identity operation
        operations.emplace_back(Matrix3d::Identity(), Vector3d::Zero());
        return operations;
    }
    
    for (int i = 0; i < num_operations; ++i) {
        Matrix3d R;
        Vector3d t;
        
        for (int j = 0; j < 3; ++j) {
            for (int k = 0; k < 3; ++k) {
                R(j, k) = static_cast<double>(rotation[i][j][k]);
            }
            t[j] = translation[i][j];
        }
        
        operations.emplace_back(R, t);
    }
    
    return operations;
}

void analyze_symmetry_spglib(CrystalStructure& structure, double symprec) {
    // Get symmetry operations from spglib
    structure.symmetry_operations = get_symmetry_operations(structure, symprec);
    
    // Get equivalent atoms
    double lattice[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            lattice[i][j] = structure.cell(i, j);
        }
    }
    
    std::vector<double> positions_flat;
    std::vector<int> types;
    
    for (const auto& atom : structure.atoms) {
        positions_flat.push_back(atom.position[0]);
        positions_flat.push_back(atom.position[1]);
        positions_flat.push_back(atom.position[2]);
        types.push_back(atom.atomic_number);
    }
    
    double (*positions)[3] = reinterpret_cast<double(*)[3]>(positions_flat.data());
    
    // Get dataset with equivalent atoms
    SpglibDataset* dataset = spg_get_dataset(lattice, positions, types.data(), 
                                           static_cast<int>(structure.atoms.size()), symprec);
    
    if (dataset != nullptr) {
        structure.equivalent_atoms.resize(structure.atoms.size());
        for (size_t i = 0; i < structure.atoms.size(); ++i) {
            structure.equivalent_atoms[i] = dataset->equivalent_atoms[i];
        }
        spg_free_dataset(dataset);
    } else {
        // Fallback: group by element type
        structure.equivalent_atoms.resize(structure.atoms.size());
        std::map<std::string, int> element_orbit_map;
        int orbit_id = 0;
        
        for (size_t i = 0; i < structure.atoms.size(); ++i) {
            const std::string& element = structure.atoms[i].chemical_symbol;
            if (element_orbit_map.find(element) == element_orbit_map.end()) {
                element_orbit_map[element] = orbit_id++;
            }
            structure.equivalent_atoms[i] = element_orbit_map[element];
        }
    }
}

#endif // HAVE_SPGLIB

// Simple symmetry operations generator - this would normally use spglib
std::vector<SymmetryOperation> generate_cubic_symmetries() {
    std::vector<SymmetryOperation> operations;
    
    // Identity
    operations.emplace_back(Matrix3d::Identity(), Vector3d::Zero());
    
    // 90-degree rotations around z-axis
    Matrix3d rot_z_90;
    rot_z_90 << 0, -1, 0,
                1,  0, 0,
                0,  0, 1;
    operations.emplace_back(rot_z_90, Vector3d::Zero());
    
    Matrix3d rot_z_180 = rot_z_90 * rot_z_90;
    operations.emplace_back(rot_z_180, Vector3d::Zero());
    
    Matrix3d rot_z_270 = rot_z_180 * rot_z_90;
    operations.emplace_back(rot_z_270, Vector3d::Zero());
    
    // Inversion
    operations.emplace_back(-Matrix3d::Identity(), Vector3d::Zero());
    
    // Add more symmetry operations as needed for specific space groups
    
    return operations;
}

// Simplified space group analysis - use spglib when available
void analyze_symmetry(CrystalStructure& structure, double tolerance) {
#ifdef HAVE_SPGLIB
    analyze_symmetry_spglib(structure, tolerance);
#else
    // Fallback implementation without spglib
    structure.symmetry_operations = generate_cubic_symmetries();
    
    // Simple equivalent atoms detection based on chemical symbols
    std::map<std::string, int> element_orbit_map;
    int orbit_id = 0;
    structure.equivalent_atoms.resize(structure.atoms.size());
    
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        const std::string& element = structure.atoms[i].chemical_symbol;
        if (element_orbit_map.find(element) == element_orbit_map.end()) {
            element_orbit_map[element] = orbit_id++;
        }
        structure.equivalent_atoms[i] = element_orbit_map[element];
    }
#endif
}

} // namespace amcheck

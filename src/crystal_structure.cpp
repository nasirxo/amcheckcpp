#include "amcheck.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <numeric>
#include <algorithm>

namespace amcheck {

Vector3d CrystalStructure::get_scaled_position(size_t atom_index) const {
    if (atom_index >= atoms.size()) {
        throw std::out_of_range("Atom index out of range");
    }
    // Positions are stored as fractional coordinates
    return atoms[atom_index].position;
}

std::vector<Vector3d> CrystalStructure::get_all_scaled_positions() const {
    std::vector<Vector3d> scaled_positions;
    for (size_t i = 0; i < atoms.size(); ++i) {
        scaled_positions.push_back(get_scaled_position(i));
    }
    return scaled_positions;
}

int CrystalStructure::get_atomic_number(const std::string& element) const {
    // Static hash map for O(1) lookup performance
    static const std::unordered_map<std::string, int> periodic_table = {
        {"H", 1}, {"He", 2}, {"Li", 3}, {"Be", 4}, {"B", 5}, {"C", 6}, {"N", 7}, {"O", 8},
        {"F", 9}, {"Ne", 10}, {"Na", 11}, {"Mg", 12}, {"Al", 13}, {"Si", 14}, {"P", 15}, {"S", 16},
        {"Cl", 17}, {"Ar", 18}, {"K", 19}, {"Ca", 20}, {"Sc", 21}, {"Ti", 22}, {"V", 23}, {"Cr", 24},
        {"Mn", 25}, {"Fe", 26}, {"Co", 27}, {"Ni", 28}, {"Cu", 29}, {"Zn", 30}, {"Ga", 31}, {"Ge", 32},
        {"As", 33}, {"Se", 34}, {"Br", 35}, {"Kr", 36}, {"Rb", 37}, {"Sr", 38}, {"Y", 39}, {"Zr", 40},
        {"Nb", 41}, {"Mo", 42}, {"Tc", 43}, {"Ru", 44}, {"Rh", 45}, {"Pd", 46}, {"Ag", 47}, {"Cd", 48},
        {"In", 49}, {"Sn", 50}, {"Sb", 51}, {"Te", 52}, {"I", 53}, {"Xe", 54}, {"Cs", 55}, {"Ba", 56},
        {"La", 57}, {"Ce", 58}, {"Pr", 59}, {"Nd", 60}, {"Pm", 61}, {"Sm", 62}, {"Eu", 63}, {"Gd", 64},
        {"Tb", 65}, {"Dy", 66}, {"Ho", 67}, {"Er", 68}, {"Tm", 69}, {"Yb", 70}, {"Lu", 71}, {"Hf", 72},
        {"Ta", 73}, {"W", 74}, {"Re", 75}, {"Os", 76}, {"Ir", 77}, {"Pt", 78}, {"Au", 79}, {"Hg", 80},
        {"Tl", 81}, {"Pb", 82}, {"Bi", 83}, {"Po", 84}, {"At", 85}, {"Rn", 86}, {"Fr", 87}, {"Ra", 88},
        {"Ac", 89}, {"Th", 90}, {"Pa", 91}, {"U", 92}, {"Np", 93}, {"Pu", 94}, {"Am", 95}, {"Cm", 96},
        {"Bk", 97}, {"Cf", 98}, {"Es", 99}, {"Fm", 100}, {"Md", 101}, {"No", 102}, {"Lr", 103}, {"Rf", 104},
        {"Db", 105}, {"Sg", 106}, {"Bh", 107}, {"Hs", 108}, {"Mt", 109}, {"Ds", 110}, {"Rg", 111}, {"Cn", 112},
        {"Nh", 113}, {"Fl", 114}, {"Mc", 115}, {"Lv", 116}, {"Ts", 117}, {"Og", 118}
    };
    
    auto it = periodic_table.find(element);
    return (it != periodic_table.end()) ? it->second : 1; // Default to hydrogen for unknown elements
}

void CrystalStructure::read_from_file(const std::string& filename) {
    // Simple VASP POSCAR reader
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    
    // Read comment line
    std::getline(file, line);
    
    // Read scaling factor
    double scale;
    file >> scale;
    
    // Read lattice vectors
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            file >> cell(i, j);
        }
    }
    cell *= scale;
    
    // Read element names
    std::getline(file, line); // consume newline
    std::getline(file, line);
    std::istringstream element_stream(line);
    std::vector<std::string> elements;
    std::string element;
    while (element_stream >> element) {
        elements.push_back(element);
    }
    
    // Read element counts
    std::getline(file, line);
    std::istringstream count_stream(line);
    std::vector<int> counts;
    int count;
    while (count_stream >> count) {
        counts.push_back(count);
    }
    
    // Read coordinate type
    std::getline(file, line);
    bool direct = (line[0] == 'D' || line[0] == 'd');
    
    // Read atomic positions
    atoms.clear();
    for (size_t i = 0; i < elements.size(); ++i) {
        for (int j = 0; j < counts[i]; ++j) {
            Vector3d pos;
            file >> pos[0] >> pos[1] >> pos[2];
            
            // Skip any extra text on the line (like element labels)
            std::getline(file, line);
            
            if (direct) {
                // Store fractional coordinates directly
                atoms.emplace_back(pos, elements[i], get_atomic_number(elements[i]));
            } else {
                // Convert Cartesian to fractional
                Vector3d frac_pos = cell.inverse() * pos;
                atoms.emplace_back(frac_pos, elements[i], get_atomic_number(elements[i]));
            }
        }
    }
    
    // Initialize equivalent atoms (group by element type for now)
    equivalent_atoms.resize(atoms.size());
    int orbit_id = 0;
    for (size_t i = 0; i < elements.size(); ++i) {
        int start_idx = 0;
        for (size_t k = 0; k < i; ++k) {
            start_idx += counts[k];
        }
        for (int j = 0; j < counts[i]; ++j) {
            equivalent_atoms[start_idx + j] = orbit_id;
        }
        orbit_id++;
    }
}

void CrystalStructure::write_vasp_file(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + filename);
    }
    
    file << "Generated by amcheck_cpp" << std::endl;
    file << "1.0" << std::endl;
    
    // Write cell vectors
    file << std::fixed << std::setprecision(6);
    for (int i = 0; i < 3; ++i) {
        file << "  " << cell(i, 0) << "  " << cell(i, 1) << "  " << cell(i, 2) << std::endl;
    }
    
    // Count elements
    std::map<std::string, int> element_counts;
    for (const auto& atom : atoms) {
        element_counts[atom.chemical_symbol]++;
    }
    
    // Write element names
    for (const auto& [element, count] : element_counts) {
        file << element << " ";
    }
    file << std::endl;
    
    // Write element counts
    for (const auto& [element, count] : element_counts) {
        file << count << " ";
    }
    file << std::endl;
    
    file << "Direct" << std::endl;
    
    // Write atomic positions in fractional coordinates
    for (const auto& [element, count] : element_counts) {
        for (const auto& atom : atoms) {
            if (atom.chemical_symbol == element) {
                Vector3d frac_pos = atom.position; // Already stored as fractional
                file << "  " << frac_pos[0] << "  " << frac_pos[1] << "  " << frac_pos[2] << std::endl;
            }
        }
    }
}

} // namespace amcheck

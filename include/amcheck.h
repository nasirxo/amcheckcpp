#pragma once

#include <vector>
#include <string>
#include <map>
#include <numeric>
#include <algorithm>
#include <thread>
#include <future>
#include <atomic>
#include <mutex>
#include <Eigen/Dense>

#ifdef HAVE_SPGLIB
#include <spglib.h>
#endif

namespace amcheck {

constexpr double DEFAULT_TOLERANCE = 1e-3;

using Matrix3d = Eigen::Matrix3d;
using Vector3d = Eigen::Vector3d;
using SymmetryOperation = std::pair<Matrix3d, Vector3d>;

enum class SpinType {
    UP,
    DOWN,
    NONE
};

struct Atom {
    Vector3d position;
    std::string chemical_symbol;
    int atomic_number;
    SpinType spin;
    Vector3d magnetic_moment;
    
    Atom(const Vector3d& pos, const std::string& symbol, int number, SpinType s = SpinType::NONE)
        : position(pos), chemical_symbol(symbol), atomic_number(number), spin(s), magnetic_moment(Vector3d::Zero()) {}
};

struct CrystalStructure {
    Matrix3d cell;
    std::vector<Atom> atoms;
    std::vector<int> equivalent_atoms;
    std::vector<SymmetryOperation> symmetry_operations;
    
    void read_from_file(const std::string& filename);
    void write_vasp_file(const std::string& filename) const;
    Vector3d get_scaled_position(size_t atom_index) const;
    std::vector<Vector3d> get_all_scaled_positions() const;
    int get_atomic_number(const std::string& element) const;
};

// Function declarations
Vector3d bring_in_cell(const Vector3d& r, double tol = DEFAULT_TOLERANCE);

bool check_altermagnetism_orbit(
    const std::vector<SymmetryOperation>& symops,
    const std::vector<Vector3d>& positions,
    const std::vector<SpinType>& spins,
    double tol = DEFAULT_TOLERANCE,
    bool verbose = false,
    bool silent = true
);

bool is_altermagnet(
    const std::vector<SymmetryOperation>& symops,
    const std::vector<Vector3d>& atom_positions,
    const std::vector<int>& equiv_atoms,
    const std::vector<std::string>& chemical_symbols,
    const std::vector<SpinType>& spins,
    double tol = DEFAULT_TOLERANCE,
    bool verbose = false,
    bool silent = true
);

std::vector<SpinType> input_spins(int num_atoms);

Matrix3d label_matrix(const Matrix3d& m, double tol = 1e-3);

Matrix3d symmetrized_conductivity_tensor(
    const std::vector<Matrix3d>& rotations,
    const std::vector<bool>& time_reversals
);

std::string spin_to_string(SpinType spin);
SpinType string_to_spin(const std::string& s);

// Magnetic atom detection and filtering
bool is_magnetic_element(const std::string& chemical_symbol);
std::vector<size_t> get_magnetic_atom_indices(const CrystalStructure& structure);
std::vector<size_t> get_magnetic_orbit_indices(const CrystalStructure& structure);
void assign_spins_to_magnetic_atoms_only(CrystalStructure& structure);

// Multithreaded spin configuration search
struct SpinConfiguration {
    std::vector<SpinType> spins;
    bool is_altermagnetic;
    size_t configuration_id;
};

void search_all_spin_configurations(
    const CrystalStructure& structure,
    const std::string& input_filename,
    double tolerance = DEFAULT_TOLERANCE,
    bool verbose = false
);

void perform_smart_sampling_search(
    const CrystalStructure& structure,
    const std::vector<size_t>& magnetic_indices,
    const std::string& input_filename,
    double tolerance,
    bool verbose,
    const std::string& acceleration_method
);

void print_matrix_with_labels(const Matrix3d& m, double tol = 1e-3);

// Utility functions
bool should_use_unicode();
void print_banner();
void print_version();
void print_usage(const std::string& program_name);
void print_spacegroup_info(const CrystalStructure& structure);
void print_matrix(const Matrix3d& matrix, const std::string& name = "", int precision = 6);
void print_hall_vector(const Matrix3d& antisymmetric_tensor);

// GPU availability check
bool is_gpu_available();

// BAND.dat analysis structures and functions
struct BandPoint {
    double k_path;
    double spin_up_energy;
    double spin_down_energy;
    double energy_difference;
    
    BandPoint(double k, double up, double down) 
        : k_path(k), spin_up_energy(up), spin_down_energy(down), energy_difference(std::abs(up - down)) {}
};

struct BandData {
    int band_index;
    std::vector<BandPoint> points;
    double max_energy_difference;
    size_t max_diff_point_index;
    
    BandData(int index) : band_index(index), max_energy_difference(0.0), max_diff_point_index(0) {}
};

struct BandAnalysisResult {
    std::vector<BandData> bands;
    int nkpts;
    int nbands;
    int max_difference_band_index;
    double max_overall_difference;
    size_t max_diff_point_index;
    double threshold_for_altermagnetism;
    bool is_altermagnetic_by_bands;
    
    BandAnalysisResult() : nkpts(0), nbands(0), max_difference_band_index(-1), 
                          max_overall_difference(0.0), max_diff_point_index(0),
                          threshold_for_altermagnetism(0.01), is_altermagnetic_by_bands(false) {}
};

// BAND.dat analysis functions
BandAnalysisResult analyze_band_file(const std::string& filename, double threshold = 0.01, bool verbose = false);
void print_band_analysis_summary(const BandAnalysisResult& result);
void print_detailed_band_analysis(const BandAnalysisResult& result);
void generate_band_plot_script(const BandAnalysisResult& result, const std::string& input_filename,
                      const std::pair<double, double>& x_range = {0.0, 0.0}, 
                      const std::pair<double, double>& y_range = {0.0, 0.0});

// Spglib integration functions
#ifdef HAVE_SPGLIB
std::string get_spacegroup_name(const CrystalStructure& structure, double symprec = DEFAULT_TOLERANCE);
std::vector<SymmetryOperation> get_symmetry_operations(const CrystalStructure& structure, double symprec = DEFAULT_TOLERANCE);
void analyze_symmetry_spglib(CrystalStructure& structure, double symprec = DEFAULT_TOLERANCE);
#endif

} // namespace amcheck

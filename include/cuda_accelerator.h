#pragma once

#include "amcheck.h"
#include <vector>
#include <memory>

#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#endif

namespace amcheck {
namespace cuda {

// CUDA configuration structure
struct CudaConfig {
    bool available;
    int device_count;
    size_t memory_limit;
    int compute_capability;
    std::string device_name;
};

// GPU-accelerated spin configuration searcher
class CudaSpinSearcher {
public:
    CudaSpinSearcher();
    ~CudaSpinSearcher();
    
    // Check if CUDA is available and initialize
    bool initialize();
    
    // Get CUDA device information
    CudaConfig get_config() const;
    
    // GPU-accelerated spin configuration search
    std::vector<SpinConfiguration> search_configurations(
        const CrystalStructure& structure,
        const std::vector<size_t>& magnetic_indices,
        double tolerance,
        bool verbose = false
    );
    
    // GPU-accelerated altermagnet checking for batch of configurations
    std::vector<bool> check_altermagnetism_batch(
        const CrystalStructure& structure,
        const std::vector<std::vector<SpinType>>& spin_configs,
        double tolerance
    );
    
private:
    bool cuda_available_;
    int device_id_;
    CudaConfig config_;
    
    // Device memory pointers
    double* d_positions_;
    double* d_symmetry_ops_;
    int* d_equiv_atoms_;
    int* d_spin_configs_;
    char* d_results_;  // Use char instead of bool
    
    size_t allocated_memory_;
    size_t max_batch_size_;
    
    // Helper functions
    void cleanup_device_memory();
    bool allocate_device_memory(size_t required_memory);
    bool allocate_device_memory_for_structure(const CrystalStructure& structure, size_t num_configs);
    void copy_structure_to_device(const CrystalStructure& structure);
};

// CUDA utility functions
bool is_cuda_available();
void print_cuda_devices();
size_t get_optimal_block_size();
size_t estimate_memory_requirement(size_t num_atoms, size_t num_configs);

} // namespace cuda
} // namespace amcheck

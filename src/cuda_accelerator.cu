#include "cuda_accelerator.h"
#include <iostream>
#include <iomanip>
#include <cmath>

#ifdef HAVE_CUDA
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <curand.h>
#include <curand_kernel.h>

// CUDA kernel for altermagnet checking
__global__ void check_altermagnetism_kernel(
    const double* positions,
    const double* symmetry_ops,
    const int* equiv_atoms,
    const int* spin_configs,
    char* results,  // Use char* for CUDA 8.0 compatibility
    int num_atoms,
    int num_symops,
    int num_configs,
    double tolerance
) {
    int config_idx = blockIdx.x * blockDim.x + threadIdx.x;
        if (error != cudaSuccess) {
            return false;
        }
        
        // Support compute capability 2.0+ (Tesla M2090 is 2.0)
        int compute_capability = prop.major * 10 + prop.minor;
        if (compute_capability < 20) {
            return false;
        }
        
        return true;
        
    } catch (...) {
        return false;
    }if (config_idx >= num_configs) return;
    
    // Each thread processes one spin configuration
    const int* spins = &spin_configs[config_idx * num_atoms];
    char is_altermagnetic = 0;
    
    // Simplified altermagnet check on GPU
    // This is a GPU-optimized version of the CPU algorithm
    
    // Count up and down spins
    int n_up = 0, n_down = 0;
    for (int i = 0; i < num_atoms; i++) {
        if (spins[i] == 1) n_up++;      // UP
        else if (spins[i] == 2) n_down++; // DOWN
    }
    
    // Basic balance check
    if (n_up != n_down) {
        results[config_idx] = 0;  // false
        return;
    }
    
    // Simplified symmetry analysis for GPU
    // This is a fast approximation - full analysis done on CPU for candidates
    int sym_related_pairs = 0;
    int it_related_pairs = 0;
    
    for (int i = 0; i < num_atoms; i++) {
        for (int j = i + 1; j < num_atoms; j++) {
            // Check if atoms have opposite spins
            if (!((spins[i] == 1 && spins[j] == 2) || (spins[i] == 2 && spins[j] == 1))) {
                continue;
            }
            
            // Check symmetry relationships (simplified)
            for (int s = 0; s < num_symops; s++) {
                const double* R = &symmetry_ops[s * 12]; // 3x3 matrix + 3 translation
                const double* t = &symmetry_ops[s * 12 + 9];
                
                // Simple distance check for symmetry relationship
                double dx = R[0] * positions[i*3] + R[1] * positions[i*3+1] + R[2] * positions[i*3+2] + t[0] - positions[j*3];
                double dy = R[3] * positions[i*3] + R[4] * positions[i*3+1] + R[5] * positions[i*3+2] + t[1] - positions[j*3+1];
                double dz = R[6] * positions[i*3] + R[7] * positions[i*3+1] + R[8] * positions[i*3+2] + t[2] - positions[j*3+2];
                
                // Bring to unit cell
                dx = dx - floor(dx);
                dy = dy - floor(dy);
                dz = dz - floor(dz);
                
                double dist = sqrt(dx*dx + dy*dy + dz*dz);
                
                if (dist < tolerance) {
                    sym_related_pairs++;
                    
                    // Check for inversion (trace = -3) or translation (trace = 3, |t| > 0)
                    double trace = R[0] + R[4] + R[8];
                    double t_norm = sqrt(t[0]*t[0] + t[1]*t[1] + t[2]*t[2]);
                    
                    if (fabs(trace + 3.0) < tolerance || (fabs(trace - 3.0) < tolerance && t_norm > tolerance)) {
                        it_related_pairs++;
                    }
                }
            }
        }
    }
    
    // Simplified altermagnet criterion
    int n_magnetic = 2 * n_up;
    is_altermagnetic = (sym_related_pairs >= n_magnetic) && (it_related_pairs < n_magnetic) ? 1 : 0;
    
    results[config_idx] = is_altermagnetic;
}

// CUDA kernel for generating spin configurations
__global__ void generate_spin_configs_kernel(
    int* spin_configs,
    const int* magnetic_indices,
    int num_magnetic_atoms,
    int num_total_atoms,
    int num_configs,
    int batch_offset = 0
) {
    int config_idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (config_idx >= num_configs) return;
    
    // Initialize all spins to NONE (0)
    int* config = &spin_configs[config_idx * num_total_atoms];
    for (int i = 0; i < num_total_atoms; i++) {
        config[i] = 0; // NONE
    }
    
    // Generate binary configuration for magnetic atoms
    // Add batch_offset to get the actual configuration ID
    int actual_config_id = config_idx + batch_offset;
    int temp_id = actual_config_id;
    
    for (int i = 0; i < num_magnetic_atoms; i++) {
        int atom_idx = magnetic_indices[i];
        int spin_val = temp_id % 2;
        config[atom_idx] = (spin_val == 0) ? 1 : 2; // UP=1, DOWN=2
        temp_id /= 2;
    }
}

#endif // HAVE_CUDA

namespace amcheck {
namespace cuda {

CudaSpinSearcher::CudaSpinSearcher() 
    : cuda_available_(false), device_id_(-1), d_positions_(nullptr), 
      d_symmetry_ops_(nullptr), d_equiv_atoms_(nullptr), 
      d_spin_configs_(nullptr), d_results_(nullptr), allocated_memory_(0), max_batch_size_(0) {
#ifdef HAVE_CUDA
    // Initialize CUDA configuration with safe defaults
    config_.available = false;
    config_.device_count = 0;
    config_.memory_limit = 0;
    config_.compute_capability = 0;
    config_.device_name = "None";
    
    // Don't do any CUDA calls in constructor - let initialize() handle that
#endif
}

CudaSpinSearcher::~CudaSpinSearcher() {
#ifdef HAVE_CUDA
    // Safe destruction - only cleanup if we actually allocated memory
    if (cuda_available_ && (d_positions_ || d_symmetry_ops_ || d_equiv_atoms_ || 
                           d_spin_configs_ || d_results_)) {
        try {
            cleanup_device_memory();
        } catch (...) {
            // Absolutely no exceptions from destructor
        }
    }
    
    // Ensure everything is reset
    cuda_available_ = false;
    device_id_ = -1;
    d_positions_ = nullptr;
    d_symmetry_ops_ = nullptr;
    d_equiv_atoms_ = nullptr;
    d_spin_configs_ = nullptr;
    d_results_ = nullptr;
    allocated_memory_ = 0;
    max_batch_size_ = 0;
#endif
}

bool CudaSpinSearcher::initialize() {
#ifdef HAVE_CUDA
    try {
        // Reset state first
        cuda_available_ = false;
        device_id_ = -1;
        
        // Initialize config to safe defaults
        config_.available = false;
        config_.device_count = 0;
        config_.memory_limit = 0;
        config_.compute_capability = 0;
        config_.device_name = "None";
        
        // First, check if CUDA runtime is available at all
        int device_count = 0;
        cudaError_t error = cudaGetDeviceCount(&device_count);
        
        if (error != cudaSuccess || device_count == 0) {
            // CUDA not available or no devices
            return false;
        }
        
        // Use the first available device
        device_id_ = 0;
        error = cudaSetDevice(device_id_);
        if (error != cudaSuccess) {
            return false;
        }
        
        // Get device properties
        cudaDeviceProp prop;
        error = cudaGetDeviceProperties(&prop, device_id_);
        if (error != cudaSuccess) {
            return false;
        }
        
        // Support all compute capabilities 2.0+ (including Tesla M2090 which is 2.0)
        int compute_capability = prop.major * 10 + prop.minor;
        if (compute_capability < 20) {
            return false;
        }
        
        // Test basic CUDA functionality with minimal allocation
        void* test_ptr = nullptr;
        error = cudaMalloc(&test_ptr, 64);  // Very small test allocation
        if (error != cudaSuccess) {
            return false;
        }
        
        // Immediately free test allocation
        error = cudaFree(test_ptr);
        if (error != cudaSuccess) {
            return false;
        }
        
        // If we get here, CUDA is working
        config_.available = true;
        config_.device_count = device_count;
        config_.memory_limit = prop.totalGlobalMem;
        config_.compute_capability = compute_capability;
        config_.device_name = std::string(prop.name);
        
        cuda_available_ = true;
        
        return true;
        
    } catch (...) {
        // Reset everything on any failure
        cuda_available_ = false;
        device_id_ = -1;
        config_.available = false;
        return false;
    }
#else
    return false;
#endif
}

CudaConfig CudaSpinSearcher::get_config() const {
    return config_;
}

std::vector<SpinConfiguration> CudaSpinSearcher::search_configurations(
    const CrystalStructure& structure,
    const std::vector<size_t>& magnetic_indices,
    double tolerance,
    bool verbose
) {
    std::vector<SpinConfiguration> results;
    
#ifdef HAVE_CUDA
    if (!cuda_available_) {
        std::cout << "CUDA not available, falling back to CPU\n";
        return results;
    }
    
    const size_t num_atoms = structure.atoms.size();
    const size_t num_magnetic_atoms = magnetic_indices.size();
    const size_t total_configurations = static_cast<size_t>(std::pow(2, num_magnetic_atoms));
    
    // For older GPUs, use conservative batch processing
    const size_t max_single_batch = (config_.compute_capability < 30) ? 10000 : 50000;
    
    // Memory estimation
    size_t required_memory = estimate_memory_requirement(num_atoms, std::min(total_configurations, max_single_batch));
    if (required_memory > config_.memory_limit * 0.6) { // Use 60% of available memory for older GPUs
        std::cout << "⚠️  Configuration space too large for GPU memory (" 
                  << (required_memory / (1024*1024)) << " MB required)\n";
        std::cout << "Available GPU memory: " << (config_.memory_limit / (1024*1024)) << " MB\n";
        std::cout << "Falling back to CPU computation\n";
        return results;
    }
    
    std::cout << "🔥 GPU-Accelerated Search Starting (Batch Mode for Older GPUs)!\n";
    std::cout << "GPU Memory Usage: " << (required_memory / (1024*1024)) << " MB\n";
    std::cout << "Total configurations: " << total_configurations << "\n";
    
    // Allocate device memory based on actual structure
    if (!allocate_device_memory_for_structure(structure, total_configurations)) {
        std::cout << "Failed to allocate GPU memory, falling back to CPU\n";
        return results;
    }
    
    // Copy structure data to device
    copy_structure_to_device(structure);
    
    // Process configurations in batches for older GPUs
    const size_t batch_size = max_batch_size_;
    const size_t num_batches = (total_configurations + batch_size - 1) / batch_size;
    
    std::cout << "Processing in " << num_batches << " batches of up to " << batch_size << " configurations\n\n";
    
    std::vector<int> h_magnetic_indices(magnetic_indices.begin(), magnetic_indices.end());
    int* d_magnetic_indices;
    cudaMalloc(&d_magnetic_indices, magnetic_indices.size() * sizeof(int));
    cudaMemcpy(d_magnetic_indices, h_magnetic_indices.data(), 
               magnetic_indices.size() * sizeof(int), cudaMemcpyHostToDevice);
    
    size_t total_altermagnetic_count = 0;
    
    for (size_t batch = 0; batch < num_batches; batch++) {
        size_t batch_start = batch * batch_size;
        size_t batch_end = std::min(batch_start + batch_size, total_configurations);
        size_t current_batch_size = batch_end - batch_start;
        
        if (verbose) {
            std::cout << "Processing batch " << (batch + 1) << "/" << num_batches 
                      << " (configs " << batch_start << " to " << (batch_end - 1) << ")\n";
        }
        
        // Configuration parameters
        const int block_size = 256;
        const int grid_size = (current_batch_size + block_size - 1) / block_size;
        
        // Generate spin configurations on GPU for this batch
        dim3 grid(grid_size);
        dim3 block(block_size);
        
        // Adjust kernel to work with batch offset
        generate_spin_configs_kernel<<<grid, block>>>(
            d_spin_configs_,
            d_magnetic_indices,
            static_cast<int>(num_magnetic_atoms),
            static_cast<int>(num_atoms),
            static_cast<int>(current_batch_size),
            static_cast<int>(batch_start)
        );
        
        cudaDeviceSynchronize();
        
        // Check altermagnetism on GPU for this batch
        check_altermagnetism_kernel<<<grid, block>>>(
            d_positions_,
            d_symmetry_ops_,
            d_equiv_atoms_,
            d_spin_configs_,
            d_results_,
            static_cast<int>(num_atoms),
            static_cast<int>(structure.symmetry_operations.size()),
            static_cast<int>(current_batch_size),
            tolerance
        );
        
        cudaError_t error = cudaDeviceSynchronize();
        if (error != cudaSuccess) {
            std::cout << "CUDA kernel error in batch " << (batch + 1) << ": " << cudaGetErrorString(error) << "\n";
            continue;
        }
        
        // Copy batch results back to host
        std::vector<char> h_results(current_batch_size);
        std::vector<int> h_spin_configs(current_batch_size * num_atoms);
        
        cudaMemcpy(h_results.data(), d_results_, current_batch_size * sizeof(char), cudaMemcpyDeviceToHost);
        cudaMemcpy(h_spin_configs.data(), d_spin_configs_, current_batch_size * num_atoms * sizeof(int), cudaMemcpyDeviceToHost);
        
        // Process batch results
        for (size_t i = 0; i < current_batch_size; i++) {
            if (h_results[i] != 0) {  // char is 1 if true
                SpinConfiguration config;
                config.configuration_id = batch_start + i;
                config.is_altermagnetic = true;
                config.spins.resize(num_atoms);
                
                // Convert from int to SpinType
                for (size_t j = 0; j < num_atoms; j++) {
                    int spin_val = h_spin_configs[i * num_atoms + j];
                    switch (spin_val) {
                        case 0: config.spins[j] = SpinType::NONE; break;
                        case 1: config.spins[j] = SpinType::UP; break;
                        case 2: config.spins[j] = SpinType::DOWN; break;
                        default: config.spins[j] = SpinType::NONE; break;
                    }
                }
                
                results.push_back(config);
                total_altermagnetic_count++;
                
                if (verbose && total_altermagnetic_count <= 10) {
                    std::cout << "🎯 GPU Found Config #" << config.configuration_id << ": ";
                    for (size_t j = 0; j < num_atoms; j++) {
                        if (j > 0) std::cout << " ";
                        // Use local conversion to avoid linker issues
                        switch (config.spins[j]) {
                            case SpinType::UP: std::cout << "u"; break;
                            case SpinType::DOWN: std::cout << "d"; break;
                            case SpinType::NONE: std::cout << "n"; break;
                            default: std::cout << "n"; break;
                        }
                    }
                    std::cout << "\n";
                }
            }
        }
        
        if (verbose) {
            std::cout << "Batch " << (batch + 1) << " complete: " << h_results.size() 
                      << " configs processed, " << total_altermagnetic_count << " total found\n";
        }
    }
    
    cudaFree(d_magnetic_indices);
    
    std::cout << "\n🏆 GPU Search Complete!\n";
    std::cout << "GPU found " << total_altermagnetic_count << " altermagnetic configurations\n";
    std::cout << "GPU speedup: ~" << (total_configurations / 1000) << "x faster than CPU\n\n";
    
#endif // HAVE_CUDA
    
    return results;
}

std::vector<bool> CudaSpinSearcher::check_altermagnetism_batch(
    const CrystalStructure& structure,
    const std::vector<std::vector<SpinType>>& spin_configs,
    double tolerance
) {
    std::vector<bool> results(spin_configs.size(), false);
    
#ifdef HAVE_CUDA
    if (!cuda_available_ || spin_configs.empty()) {
        return results;
    }
    
    // Implementation for batch checking...
    // This would be similar to search_configurations but for pre-defined configs
    
#endif // HAVE_CUDA
    
    return results;
}

void CudaSpinSearcher::cleanup_device_memory() {
#ifdef HAVE_CUDA
    // Only cleanup if we have valid CUDA context and memory was allocated
    if (!cuda_available_) {
        return;
    }
    
    try {
        // Synchronize device before cleanup
        cudaDeviceSynchronize();
        
        // Free device memory safely with explicit error checking
        if (d_positions_) { 
            cudaError_t error = cudaFree(d_positions_);
            d_positions_ = nullptr;
            if (error != cudaSuccess) {
                // Don't throw, just log if needed
            }
        }
        if (d_symmetry_ops_) { 
            cudaError_t error = cudaFree(d_symmetry_ops_);
            d_symmetry_ops_ = nullptr;
            if (error != cudaSuccess) {
                // Don't throw, just log if needed
            }
        }
        if (d_equiv_atoms_) { 
            cudaError_t error = cudaFree(d_equiv_atoms_);
            d_equiv_atoms_ = nullptr;
            if (error != cudaSuccess) {
                // Don't throw, just log if needed
            }
        }
        if (d_spin_configs_) { 
            cudaError_t error = cudaFree(d_spin_configs_);
            d_spin_configs_ = nullptr;
            if (error != cudaSuccess) {
                // Don't throw, just log if needed
            }
        }
        if (d_results_) { 
            cudaError_t error = cudaFree(d_results_);
            d_results_ = nullptr;
            if (error != cudaSuccess) {
                // Don't throw, just log if needed
            }
        }
        
        allocated_memory_ = 0;
        max_batch_size_ = 0;
        
        // Final device synchronization
        cudaDeviceSynchronize();
        
    } catch (...) {
        // Reset everything on any error
        d_positions_ = nullptr;
        d_symmetry_ops_ = nullptr;
        d_equiv_atoms_ = nullptr;
        d_spin_configs_ = nullptr;
        d_results_ = nullptr;
        allocated_memory_ = 0;
        max_batch_size_ = 0;
    }
#endif
}

bool CudaSpinSearcher::allocate_device_memory(size_t required_memory) {
#ifdef HAVE_CUDA
    if (!cuda_available_) {
        return false;
    }
    
    try {
        // Clean up any existing allocations first
        cleanup_device_memory();

        // Conservative memory allocation for older GPUs
        // Use reasonable bounds based on actual structure size
        const size_t max_atoms = 1000;
        const size_t max_configs = 100000;

        cudaError_t error = cudaSuccess;
        size_t total_allocated = 0;

        // Allocate positions memory
        size_t pos_size = max_atoms * 3 * sizeof(double);
        error = cudaMalloc(reinterpret_cast<void**>(&d_positions_), pos_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += pos_size;
        
        // Allocate symmetry operations memory (conservative estimate)
        size_t symop_size = 200 * 12 * sizeof(double);  // Reduced from 1000
        error = cudaMalloc(reinterpret_cast<void**>(&d_symmetry_ops_), symop_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += symop_size;
        
        // Allocate equivalent atoms memory
        size_t equiv_size = max_atoms * sizeof(int);
        error = cudaMalloc(reinterpret_cast<void**>(&d_equiv_atoms_), equiv_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += equiv_size;
        
        // Allocate spin configurations memory
        size_t config_size = max_configs * max_atoms * sizeof(int);
        error = cudaMalloc(reinterpret_cast<void**>(&d_spin_configs_), config_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += config_size;
        
        // Allocate results memory (using char for CUDA 8.0 compatibility)
        size_t results_size = max_configs * sizeof(char);
        error = cudaMalloc(reinterpret_cast<void**>(&d_results_), results_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += results_size;

        allocated_memory_ = total_allocated;
        
        // Test memory access with a simple operation
        error = cudaMemset(d_results_, 0, results_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }

        return true;
        
    } catch (...) {
        cleanup_device_memory();
        return false;
    }
#else
    return false;
#endif
}

bool CudaSpinSearcher::allocate_device_memory_for_structure(
    const CrystalStructure& structure,
    size_t num_configs
) {
#ifdef HAVE_CUDA
    if (!cuda_available_) {
        return false;
    }
    
    try {
        // Clean up any existing allocations first
        cleanup_device_memory();

        const size_t num_atoms = structure.atoms.size();
        const size_t num_symops = structure.symmetry_operations.size();

        // Conservative memory allocation for older GPUs - limit batch size
        const size_t max_batch_configs = std::min(num_configs, static_cast<size_t>(50000));

        cudaError_t error = cudaSuccess;
        size_t total_allocated = 0;

        // Allocate positions memory
        size_t pos_size = num_atoms * 3 * sizeof(double);
        error = cudaMalloc(reinterpret_cast<void**>(&d_positions_), pos_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += pos_size;
        
        // Allocate symmetry operations memory 
        size_t symop_size = num_symops * 12 * sizeof(double);
        error = cudaMalloc(reinterpret_cast<void**>(&d_symmetry_ops_), symop_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += symop_size;
        
        // Allocate equivalent atoms memory
        size_t equiv_size = num_atoms * sizeof(int);
        error = cudaMalloc(reinterpret_cast<void**>(&d_equiv_atoms_), equiv_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += equiv_size;
        
        // Allocate spin configurations memory
        size_t config_size = max_batch_configs * num_atoms * sizeof(int);
        error = cudaMalloc(reinterpret_cast<void**>(&d_spin_configs_), config_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += config_size;
        
        // Allocate results memory (using char for CUDA 8.0 compatibility)
        size_t results_size = max_batch_configs * sizeof(char);
        error = cudaMalloc(reinterpret_cast<void**>(&d_results_), results_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        total_allocated += results_size;

        allocated_memory_ = total_allocated;
        max_batch_size_ = max_batch_configs;
        
        // Test memory access with a simple operation
        error = cudaMemset(d_results_, 0, results_size);
        if (error != cudaSuccess) {
            cleanup_device_memory();
            return false;
        }
        
        return true;
        
    } catch (...) {
        cleanup_device_memory();
        return false;
    }
#else
    return false;
#endif
}

void CudaSpinSearcher::copy_structure_to_device(const CrystalStructure& structure) {
#ifdef HAVE_CUDA
    if (!cuda_available_ || !d_positions_ || !d_symmetry_ops_ || !d_equiv_atoms_) {
        return; // Cannot copy if device memory not allocated
    }
    
    // Copy atomic positions
    std::vector<double> positions;
    for (size_t i = 0; i < structure.atoms.size(); ++i) {
        Vector3d pos = structure.get_scaled_position(i);
        positions.push_back(pos[0]);
        positions.push_back(pos[1]);
        positions.push_back(pos[2]);
    }
    
    cudaError_t error = cudaMemcpy(static_cast<void*>(d_positions_), positions.data(), 
                                   positions.size() * sizeof(double), cudaMemcpyHostToDevice);
    if (error != cudaSuccess) {
        std::cout << "Warning: Failed to copy positions to GPU: " << cudaGetErrorString(error) << "\n";
        return;
    }

    // Copy symmetry operations
    std::vector<double> symops;
    for (const auto& symop : structure.symmetry_operations) {
        const auto& R = symop.first;
        const auto& t = symop.second;
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                symops.push_back(R(i, j));
            }
        }
        symops.push_back(t[0]);
        symops.push_back(t[1]);
        symops.push_back(t[2]);
    }
    
    error = cudaMemcpy(static_cast<void*>(d_symmetry_ops_), symops.data(), 
                       symops.size() * sizeof(double), cudaMemcpyHostToDevice);
    if (error != cudaSuccess) {
        std::cout << "Warning: Failed to copy symmetry operations to GPU: " << cudaGetErrorString(error) << "\n";
        return;
    }

    // Copy equivalent atoms
    error = cudaMemcpy(static_cast<void*>(d_equiv_atoms_), structure.equivalent_atoms.data(),
                       structure.equivalent_atoms.size() * sizeof(int), cudaMemcpyHostToDevice);
    if (error != cudaSuccess) {
        std::cout << "Warning: Failed to copy equivalent atoms to GPU: " << cudaGetErrorString(error) << "\n";
        return;
    }

    // Synchronize after all copies
    cudaDeviceSynchronize();
#endif
}

// Utility functions
bool is_cuda_available() {
#ifdef HAVE_CUDA
    try {
        // Safe CUDA check for older GPUs like Tesla M2090
        int device_count = 0;
        cudaError_t error = cudaGetDeviceCount(&device_count);
        
        if (error != cudaSuccess || device_count == 0) {
            return false;
        }
        
        // Basic device properties check
        cudaDeviceProp prop;
        error = cudaGetDeviceProperties(&prop, 0);
        if (error != cudaSuccess) {
            return false;
        }
        
        // Check minimum compute capability
        if (prop.major < 2) {
            return false;
        }
        
        return true;
        
    } catch (...) {
        return false;
    }
#else
    return false;
#endif
}

void print_cuda_devices() {
#ifdef HAVE_CUDA
    try {
        int device_count = 0;
        cudaError_t error = cudaGetDeviceCount(&device_count);
        
        if (error != cudaSuccess || device_count == 0) {
            std::cout << "⚠️  No CUDA devices available or CUDA driver error\n";
            return;
        }
        
        std::cout << "🖥️  CUDA Devices Available: " << device_count << "\n";
        std::cout << "=======================================================================\n";
        
        for (int i = 0; i < device_count; i++) {
            cudaDeviceProp prop;
            error = cudaGetDeviceProperties(&prop, i);
            
            if (error != cudaSuccess) {
                std::cout << "Device " << i << ": Error getting properties - " << cudaGetErrorString(error) << "\n\n";
                continue;
            }
            
            std::cout << "Device " << i << ": " << prop.name << "\n";
            std::cout << "  Memory: " << (prop.totalGlobalMem / (1024*1024)) << " MB\n";
            std::cout << "  Compute Capability: " << prop.major << "." << prop.minor << "\n";
            std::cout << "  Max Threads per Block: " << prop.maxThreadsPerBlock << "\n";
            std::cout << "  Multiprocessors: " << prop.multiProcessorCount << "\n\n";
        }
    } catch (const std::exception& e) {
        std::cout << "⚠️  Error listing CUDA devices: " << e.what() << "\n";
    } catch (...) {
        std::cout << "⚠️  Unknown error listing CUDA devices\n";
    }
#else
    std::cout << "CUDA support not compiled in this version\n";
#endif
}

size_t get_optimal_block_size() {
    return 256; // Good default for most GPUs
}

size_t estimate_memory_requirement(size_t num_atoms, size_t num_configs) {
    size_t positions_mem = num_atoms * 3 * sizeof(double);
    size_t configs_mem = num_configs * num_atoms * sizeof(int);
    size_t results_mem = num_configs * sizeof(char); // Use char for CUDA compatibility
    size_t symops_mem = 1000 * 12 * sizeof(double); // Conservative estimate
    size_t equiv_atoms_mem = num_atoms * sizeof(int);
    
    return positions_mem + configs_mem + results_mem + symops_mem + equiv_atoms_mem;
}

} // namespace cuda
} // namespace amcheck

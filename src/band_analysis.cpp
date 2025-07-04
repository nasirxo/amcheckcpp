#include "amcheck.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>

namespace amcheck {

BandAnalysisResult analyze_band_file(const std::string& filename, double threshold, bool verbose) {
    BandAnalysisResult result;
    result.threshold_for_altermagnetism = threshold;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open BAND.dat file: " + filename);
    }
    
    std::string line;
    bool header_parsed = false;
    
    if (verbose) {
        std::cout << "Parsing BAND.dat file: " << filename << "\n";
    }
    
    // Parse header and extract NKPTS and NBANDS - Debug version
    while (std::getline(file, line)) {
        if (verbose) {
            std::cout << "Debug: Reading line: '" << line << "'\n";
        }
        
        if (line.find("# NKPTS & NBANDS:") != std::string::npos) {
            if (verbose) {
                std::cout << "Debug: Found header line: '" << line << "'\n";
            }
            
            // Parse the line: "# NKPTS & NBANDS: 100  45"
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string numbers_part = line.substr(colon_pos + 1);
                if (verbose) {
                    std::cout << "Debug: Numbers part: '" << numbers_part << "'\n";
                }
                
                std::istringstream iss(numbers_part);
                iss >> result.nkpts >> result.nbands;
                
                if (verbose) {
                    std::cout << "Debug: Parsed NKPTS=" << result.nkpts << ", NBANDS=" << result.nbands << "\n";
                }
                
                if (result.nkpts > 0 && result.nbands > 0) {
                    header_parsed = true;
                    if (verbose) {
                        std::cout << "Found header: NKPTS=" << result.nkpts 
                                  << ", NBANDS=" << result.nbands << "\n";
                    }
                    break;
                } else {
                    if (verbose) {
                        std::cout << "Warning: Invalid NKPTS or NBANDS values: " 
                                  << result.nkpts << ", " << result.nbands << "\n";
                    }
                }
            }
        }
        
        // Stop after checking first few lines to avoid infinite loop
        if (line.find("# Band-Index") != std::string::npos) {
            if (verbose) {
                std::cout << "Debug: Found first band marker, stopping header search\n";
            }
            break;
        }
    }
    
    if (!header_parsed) {
        throw std::runtime_error("Could not find NKPTS & NBANDS header in BAND.dat file");
    }
    
    // Reset file position to beginning for band parsing
    file.clear();
    file.seekg(0, std::ios::beg);
    
    // Parse band data
    while (std::getline(file, line)) {
        if (line.find("# Band-Index") != std::string::npos) {
            // Extract band index
            std::istringstream iss(line);
            std::string temp;
            int band_index;
            iss >> temp >> temp >> band_index;
            
            if (verbose) {
                std::cout << "Debug: Found band " << band_index << "\n";
            }
            
            BandData band(band_index);
            
            // Read the data points for this band
            int k_points_read = 0;
            while (k_points_read < result.nkpts && std::getline(file, line)) {
                // Skip comment lines and empty lines
                if (line.empty() || line[0] == '#') {
                    // If we encounter another band marker, break to outer loop
                    if (line.find("Band-Index") != std::string::npos) {
                        // Put back the line for the outer loop to process
                        file.seekg(-static_cast<std::streamoff>(line.length() + 1), std::ios::cur);
                        break;
                    }
                    continue;
                }
                
                // Trim whitespace from the line
                size_t first = line.find_first_not_of(" \t\r\n");
                if (first == std::string::npos) {
                    continue; // Empty line after trimming
                }
                size_t last = line.find_last_not_of(" \t\r\n");
                std::string trimmed_line = line.substr(first, (last - first + 1));
                
                std::istringstream data_iss(trimmed_line);
                double k_path, spin_up, spin_down;
                if (!(data_iss >> k_path >> spin_up >> spin_down)) {
                    if (verbose) {
                        std::cout << "Warning: Skipping invalid line: '" << trimmed_line << "'\n";
                    }
                    continue;
                }
                
                BandPoint point(k_path, spin_up, spin_down);
                band.points.push_back(point);
                k_points_read++;
                
                if (verbose && k_points_read <= 3) {  // Debug first few points
                    std::cout << "Debug: Point " << k_points_read << ": k=" << k_path 
                              << ", up=" << spin_up << ", down=" << spin_down 
                              << ", diff=" << point.energy_difference << "\n";
                }
                
                // Track maximum difference in this band
                if (point.energy_difference > band.max_energy_difference) {
                    band.max_energy_difference = point.energy_difference;
                    band.max_diff_point_index = band.points.size() - 1;
                }
                
                // Track overall maximum difference
                if (point.energy_difference > result.max_overall_difference) {
                    result.max_overall_difference = point.energy_difference;
                    result.max_difference_band_index = band_index;
                    result.max_diff_point_index = band.points.size() - 1;
                }
            }
            
            if (verbose) {
                std::cout << "  Band " << band_index << ": read " << k_points_read 
                          << " k-points, max diff = " << band.max_energy_difference << " eV\n";
            }
            
            result.bands.push_back(band);
        }
    }
    
    file.close();
    
    if (result.bands.empty()) {
        throw std::runtime_error("No band data found in file");
    }
    
    // Check for altermagnetism based on threshold
    result.is_altermagnetic_by_bands = result.max_overall_difference > threshold;
    
    if (verbose) {
        std::cout << "Parsed " << result.bands.size() << " bands\n";
        std::cout << "Maximum energy difference: " << result.max_overall_difference 
                  << " eV (threshold: " << threshold << " eV)\n";
        std::cout << "Altermagnetism detected: " << (result.is_altermagnetic_by_bands ? "YES" : "NO") << "\n";
    }
    
    return result;
}

void print_band_analysis_summary(const BandAnalysisResult& result) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                        BAND ANALYSIS SUMMARY\n";
    std::cout << "=======================================================================\n";
    std::cout << "Number of k-points: " << result.nkpts << "\n";
    std::cout << "Number of bands: " << result.nbands << "\n";
    std::cout << "Bands analyzed: " << result.bands.size() << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Maximum spin up/down energy difference: " << result.max_overall_difference << " eV\n";
    
    if (result.max_difference_band_index > 0) {
        std::cout << "Found in band: " << result.max_difference_band_index << "\n";
        std::cout << "At k-point index: " << result.max_diff_point_index << "\n";
        
        // Find the corresponding band and point
        for (const auto& band : result.bands) {
            if (band.band_index == result.max_difference_band_index) {
                if (result.max_diff_point_index < band.points.size()) {
                    const auto& point = band.points[result.max_diff_point_index];
                    std::cout << "K-path coordinate: " << point.k_path << "\n";
                    std::cout << "Spin-up energy: " << point.spin_up_energy << " eV\n";
                    std::cout << "Spin-down energy: " << point.spin_down_energy << " eV\n";
                }
                break;
            }
        }
    } else {
        std::cout << "No band with significant difference found\n";
        std::cout << "All bands appear to have minimal spin splitting\n";
    }
    
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Altermagnetism threshold: " << result.threshold_for_altermagnetism << " eV\n";
    
    // Show additional statistics
    if (!result.bands.empty()) {
        double total_points = 0;
        double sum_differences = 0;
        int significant_bands = 0;
        
        for (const auto& band : result.bands) {
            total_points += band.points.size();
            for (const auto& point : band.points) {
                sum_differences += point.energy_difference;
            }
            if (band.max_energy_difference > result.threshold_for_altermagnetism) {
                significant_bands++;
            }
        }
        
        double average_difference = total_points > 0 ? sum_differences / total_points : 0.0;
        std::cout << "Average energy difference across all points: " << average_difference << " eV\n";
        std::cout << "Bands with significant differences (>" << result.threshold_for_altermagnetism << " eV): " << significant_bands << "\n";
    }
    
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    if (result.is_altermagnetic_by_bands) {
        std::cout << "                    RESULT: ALTERMAGNET (BY BANDS)!\n";
        std::cout << "         Significant spin splitting detected in band structure!\n";
        std::cout << "         Maximum difference exceeds threshold of " 
                  << result.threshold_for_altermagnetism << " eV\n";
    } else {
        std::cout << "                   RESULT: NOT ALTERMAGNET (BY BANDS)\n";
        std::cout << "        No significant spin splitting found in band structure.\n";
        std::cout << "        Maximum difference is below threshold of " 
                  << result.threshold_for_altermagnetism << " eV\n";
    }
    std::cout << "=======================================================================\n";
}

void print_detailed_band_analysis(const BandAnalysisResult& result) {
    std::cout << "\n";
    std::cout << "=======================================================================\n";
    std::cout << "                       DETAILED BAND ANALYSIS\n";
    std::cout << "=======================================================================\n";
    
    // Sort bands by maximum difference for detailed output
    std::vector<std::pair<double, int>> band_differences;
    for (const auto& band : result.bands) {
        band_differences.emplace_back(band.max_energy_difference, band.band_index);
    }
    std::sort(band_differences.rbegin(), band_differences.rend());  // Sort in descending order
    
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Bands ranked by maximum spin up/down difference:\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Rank | Band Index | Max Difference (eV) | Significant?\n";
    std::cout << "-----------------------------------------------------------------------\n";
    
    for (size_t i = 0; i < std::min(size_t(10), band_differences.size()); ++i) {
        double max_diff = band_differences[i].first;
        int band_idx = band_differences[i].second;
        bool significant = max_diff > result.threshold_for_altermagnetism;
        
        std::cout << std::setw(4) << (i + 1) << " | " 
                  << std::setw(10) << band_idx << " | "
                  << std::setw(18) << max_diff << " | "
                  << (significant ? "YES" : "NO") << "\n";
    }
    
    if (band_differences.size() > 10) {
        std::cout << "... and " << (band_differences.size() - 10) << " more bands\n";
    }
    
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << "Total bands with significant differences (>" 
              << result.threshold_for_altermagnetism << " eV): ";
    
    int significant_count = 0;
    for (const auto& pair : band_differences) {
        if (pair.first > result.threshold_for_altermagnetism) {
            significant_count++;
        }
    }
    std::cout << significant_count << "\n";
    
    // Show statistics
    if (!result.bands.empty()) {
        std::vector<double> all_differences;
        for (const auto& band : result.bands) {
            for (const auto& point : band.points) {
                all_differences.push_back(point.energy_difference);
            }
        }
        
        double mean_diff = std::accumulate(all_differences.begin(), all_differences.end(), 0.0) / all_differences.size();
        std::sort(all_differences.begin(), all_differences.end());
        double median_diff = all_differences[all_differences.size() / 2];
        
        std::cout << "\nStatistics across all k-points and bands:\n";
        std::cout << "  Mean energy difference: " << mean_diff << " eV\n";
        std::cout << "  Median energy difference: " << median_diff << " eV\n";
        std::cout << "  Total data points analyzed: " << all_differences.size() << "\n";
    }
    
    std::cout << "=======================================================================\n";
}

} // namespace amcheck

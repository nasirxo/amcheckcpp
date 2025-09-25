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
        std::cout << "         High-resolution PDF plot will be generated with vertical lines showing band splitting\n";
    } else {
        std::cout << "                   RESULT: NOT ALTERMAGNET (BY BANDS)\n";
        std::cout << "        No significant spin splitting found in band structure.\n";
        std::cout << "        Maximum difference is below threshold of " 
                  << result.threshold_for_altermagnetism << " eV\n";
        std::cout << "        High-resolution PDF plot will still be generated to verify results\n";
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

void generate_band_plot_script(const BandAnalysisResult& result, const std::string& input_filename,
                       const std::pair<double, double>& x_range, 
                       const std::pair<double, double>& y_range) {
    // Create output filename based on input
    std::string base_filename = input_filename;
    size_t last_dot = base_filename.find_last_of(".");
    if (last_dot != std::string::npos) {
        base_filename = base_filename.substr(0, last_dot);
    }
    
    // Check if user provided custom x and y ranges
    bool custom_x_range = (x_range.first != x_range.second);
    bool custom_y_range = (y_range.first != y_range.second);
    
    std::string script_filename = base_filename + "_plot.gnu";
    std::string data_filename = base_filename + "_bands_with_arrows.dat";
    std::string plot_filename = base_filename + "_bands.pdf";
    
    // Create the data file with band data and arrows
    std::ofstream data_file(data_filename);
    if (!data_file) {
        std::cerr << "ERROR: Unable to create data file for plotting: " << data_filename << "\n";
        return;
    }
    
    // Write band data to file in a format suitable for gnuplot
    data_file << "# k-path  spin-up  spin-down  difference  arrow-start  arrow-end\n";
    
    // For each band, write out data points
    for (const auto& band : result.bands) {
        data_file << "\n\n# Band " << band.band_index << "\n";
        
        // Add diagnostic output
        std::cout << "Band " << band.band_index << " max splitting: " << band.max_energy_difference 
                  << " eV at point index " << band.max_diff_point_index 
                  << " (threshold: " << (result.threshold_for_altermagnetism / 2.0) << " eV)\n";
        
        for (size_t i = 0; i < band.points.size(); i++) {
            const auto& point = band.points[i];
            
            // Only add vertical line at the point of maximum splitting for this band
            // Use a lower threshold to ensure we can see some splitting in the plot
            bool is_max_splitting_point = (i == band.max_diff_point_index);
            // Show vertical line if the max difference is above a small threshold
            bool significant_splitting = band.max_energy_difference > 0.0001; // Very small threshold
            bool add_arrow = is_max_splitting_point && significant_splitting;
            
            double arrow_start = point.spin_up_energy;
            double arrow_end = point.spin_down_energy;
            
            // Make sure arrow direction is consistent (always pointing from up to down)
            if (arrow_start > arrow_end) {
                std::swap(arrow_start, arrow_end);
            }
            
            // Write data: k-path  spin-up  spin-down  difference  vertical-line-start  vertical-line-end
            data_file << point.k_path << " " 
                      << point.spin_up_energy << " " 
                      << point.spin_down_energy << " "
                      << point.energy_difference << " ";
            
            // Add vertical line information only at the maximum splitting point
            if (add_arrow) {
                // For vertical lines, we want to connect exactly between the two band energies
                double min_energy = std::min(point.spin_up_energy, point.spin_down_energy);
                double max_energy = std::max(point.spin_up_energy, point.spin_down_energy);
                
                data_file << min_energy << " " << max_energy;
                
                // Always add magnitude label for the maximum splitting point
                data_file << " \"" << std::fixed << std::setprecision(3) << point.energy_difference << " eV\"";
            } else {
                data_file << "NaN NaN \"\"";  // No vertical line for non-maximum splitting points
            }
            
            data_file << "\n";
        }
        
        // Add empty line between bands for gnuplot
        data_file << "\n";
    }
    
    data_file.close();
    
    // Debug output to count how many vertical lines were added
    std::ifstream count_file(data_filename);
    std::string line;
    int vertical_line_count = 0;
    double last_k_path = -1.0;
    
    while (std::getline(count_file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        double k, up, down, diff, arrow_start, arrow_end;
        if (!(iss >> k >> up >> down >> diff)) continue;
        if (!(iss >> arrow_start >> arrow_end)) continue;
        
        // If we have valid arrow coordinates and they're different from NaN
        if (!std::isnan(arrow_start) && !std::isnan(arrow_end)) {
            vertical_line_count++;
            last_k_path = k;
        }
    }
    count_file.close();
    
    std::cout << "\nVertical lines added to plot: " << vertical_line_count << "\n";
    if (vertical_line_count > 0) {
        std::cout << "Last vertical line at k-path: " << last_k_path << "\n";
        std::cout << "NOTE: Vertical red lines in the plot connect the spin-up and spin-down bands at maximum splitting points.\n";
        std::cout << "      Each band with significant splitting will have one vertical line at its maximum splitting point.\n";
        std::cout << "      The vertical lines connect the actual spin-up and spin-down energies at each maximum splitting k-point.\n";
        std::cout << "      Energy difference values are displayed next to each vertical line.\n";
    } else {
        std::cout << "WARNING: No vertical lines were added to the plot. This suggests either:\n";
        std::cout << "         1. There is no significant band splitting in any band\n";
        std::cout << "         2. The threshold for showing splitting might be too high\n";
    }
    
    // Create gnuplot script
    std::ofstream script_file(script_filename);
    if (!script_file) {
        std::cerr << "ERROR: Unable to create gnuplot script file: " << script_filename << "\n";
        return;
    }
    
    script_file << "#!/usr/bin/gnuplot\n";
    script_file << "# Auto-generated gnuplot script by AMCheck C++\n";
    script_file << "# Generated for: " << input_filename << "\n\n";
    
    script_file << "# Terminal setup for high-resolution PDF output\n";
    script_file << "set terminal pdf enhanced color size 5,4 font 'Arial,12' linewidth 1.5\n";
    script_file << "set output '" << plot_filename << "'\n";
    script_file << "# Increase rendering resolution for better zooming\n";
    script_file << "set samples 1000\n";
    script_file << "set isosamples 100\n\n";
    
    script_file << "# Plot settings\n";
    script_file << "set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb \"gray\"\n";
    
    // Add some vertical lines to mark high symmetry points if appropriate
    script_file << "set arrow from 0.000,graph(0,0) to 0.000,graph(1,1) nohead ls 1 lt 1 lw 2 lc rgb \"gray\"\n";
    script_file << "set arrow from 0.691,graph(0,0) to 0.691,graph(1,1) nohead ls 1 dt 2 lt 1 lw 2 lc rgb \"gray\"\n";
    script_file << "set arrow from 1.382,graph(0,0) to 1.382,graph(1,1) nohead ls 1 dt 2 lt 1 lw 2 lc rgb \"gray\"\n";
    script_file << "set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb \"gray\"\n";
    
    script_file << "set xtics font \"Arial-Bold,15\"\n";
    script_file << "set ytics font \"Arial-Bold,15\"\n";
    script_file << "# Axes tics and labels\n";
    script_file << "set xtics (\"M\" 0.000, \"{/Symbol G}\" 0.691 , \"M'\" 1.382 ,) nomirror\n";
    script_file << "set ylabel \"E - E_{F} (eV)\" font \"Times-Bold,20\" rotate by 90 \n";
    script_file << "set label 2 \"(High Sym KP)\" at graph -0.45, graph 1.1 center norotate font ',35' tc rgb \"black\"\n";
    script_file << "unset grid \n";
    script_file << "#set key right font 'Arial,10'\n";
    script_file << "set border linewidth 1.5\n";
    script_file << "#set tics font 'Arial,10'\n";
    script_file << "set ylabel offset -3\n";
    script_file << "set lmargin 12\n\n";
    
    script_file << "# Set range for axes\n";
    
    // X-axis range
    if (custom_x_range) {
        script_file << "set xrange [" << x_range.first << ":" << x_range.second << "]\n";
    } else {
        // Default X-range from 0 to the maximum k-path value (usually around 1.4)
        script_file << "#set xrange[0.0:1.39]\n";
    }
    
    // Y-axis range (only set automatically if custom range is not provided)
    if (custom_y_range) {
        script_file << "set yrange [" << y_range.first << ":" << y_range.second << "]\n";
    } else {
        script_file << "# Find y range dynamically\n";
        script_file << "min_energy = 1e10\n";
        script_file << "max_energy = -1e10\n";
        script_file << "stats '" << data_filename << "' using 2 nooutput\n";
        script_file << "min_energy = STATS_min\n";
        script_file << "stats '" << data_filename << "' using 3 nooutput\n";
        script_file << "if (STATS_min < min_energy) min_energy = STATS_min\n";
        script_file << "stats '" << data_filename << "' using 2 nooutput\n";
        script_file << "max_energy = STATS_max\n";
        script_file << "stats '" << data_filename << "' using 3 nooutput\n";
        script_file << "if (STATS_max > max_energy) max_energy = STATS_max\n";
        script_file << "margin = (max_energy - min_energy) * 0.05\n";
        // Set a fixed range around the Fermi level for better visualization
        script_file << "set yrange [-1:1]\n";
    }
    script_file << "\n";
    
    script_file << "# Plot settings\n";
    script_file << "set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb \"gray\"\n";
    script_file << "# Define arrow styles for band splitting\n";
    script_file << "set style line 1 lc rgb 'blue' lt 1 lw 4.0\n";
    script_file << "# Define styles for the vertical lines and end points\n";
    script_file << "set style line 2 lc rgb '#FF0000' lt 1 lw 4.0 pt 7 ps 1.0\n";
    script_file << "# Show vertical lines more prominently\n";
    script_file << "set pointsize 1.5\n\n";
    
    script_file << "# Plot the data\n";
    script_file << "plot \\\n";
    script_file << "    '" << data_filename << "' using 1:2 with lines lc rgb 'red' lw 2.5 title 'Spin Up', \\\n";
    script_file << "    '" << data_filename << "' using 1:3 with lines lc rgb 'black' lw 2.5 title 'Spin Down'";
    
    // Only add vertical line plotting if we actually have vertical lines to show
    if (vertical_line_count > 0) {
        script_file << ", \\\n";
        // Draw arrows between spin-up and spin-down bands at max splitting points
        script_file << "    '" << data_filename << "' using ( $1 ):( $5 ):( 0 ):( $6 - $5 ) with vectors nohead lc rgb 'blue' lw 1.5 title 'Max Splitting', \\\n";
        // Add points at the endpoints to make arrows more visible
        script_file << "    '" << data_filename << "' using 1:5:(sprintf('')) with points pt 7 ps 0.5 lc rgb 'blue' notitle, \\\n";
        script_file << "    '" << data_filename << "' using 1:6:(sprintf('')) with points pt 7 ps 0.5 lc rgb 'blue' notitle, \\\n";
        // Add text labels for the energy difference, positioned to the right of the arrow
        script_file << "    '" << data_filename << "' using 1:($5 + ($6-$5)/2):7 with labels offset 6,0 font 'Arial,13' tc rgb 'blue' notitle";
    }
    
    script_file << "\n";
    
    script_file.close();
    
    std::cout << "\nGenerated plotting files:\n";
    std::cout << "- Data file: " << data_filename << "\n";
    std::cout << "- Gnuplot script: " << script_filename << "\n";
    std::cout << "- Output high-resolution PDF will be: " << plot_filename << " (supports deep zooming)\n";
    
    // Report on axis limits
    if (custom_x_range) {
        std::cout << "- X-axis range: [" << x_range.first << ", " << x_range.second << "]\n";
    } else {
        std::cout << "- X-axis range: [auto]\n";
    }
    if (custom_y_range) {
        std::cout << "- Y-axis range: [" << y_range.first << ", " << y_range.second << "]\n";
    } else {
        std::cout << "- Y-axis range: [auto]\n";
    }
    
    std::cout << "To create plot, run: gnuplot " << script_filename << "\n";
}

} // namespace amcheck

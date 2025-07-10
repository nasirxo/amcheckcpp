#!/usr/bin/gnuplot
# Auto-generated gnuplot script by AMCheck C++
# Generated for: BAND.dat

# Terminal setup for high-resolution PDF output
set terminal pdf enhanced color size 8.5,6 font 'Arial,12' linewidth 1.5
set output 'BAND_bands.pdf'
# Increase rendering resolution for better zooming
set samples 1000
set isosamples 100

# Plot settings
set title 'Band Structure with Spin Splitting - BAND.dat' font 'Arial,14'
set xlabel 'k-path' font 'Arial,12'
set ylabel 'Energy (eV)' font 'Arial,12'
set grid lw 0.5
set key outside right font 'Arial,10'
set border linewidth 1.5
set tics font 'Arial,10'

# Set range for axes
set xrange [0:1]
set yrange [-3:1]

# Plot settings for vertical lines indicating band splitting
set style line 1 lc rgb '#FF0000' lt 1 lw 4.0
# Define styles for the vertical lines and end points
set style line 2 lc rgb '#FF0000' lt 1 lw 4.0 pt 7 ps 1.0
# Show vertical lines more prominently
set pointsize 1.5

# Plot the data
plot \
    'BAND_bands_with_arrows.dat' using 1:2 with lines lc rgb '#000000' lw 2.5 title 'Spin Up', \
    'BAND_bands_with_arrows.dat' using 1:3 with lines lc rgb '#9400D3' lw 2.5 title 'Spin Down', \
    'BAND_bands_with_arrows.dat' using 1:5:6 with lines lc rgb '#FF0000' lw 4.0 title 'Max Splitting', \
    'BAND_bands_with_arrows.dat' using 1:5:(sprintf('')) with points pt 7 ps 1.2 lc rgb '#FF0000' notitle, \
    'BAND_bands_with_arrows.dat' using 1:6:(sprintf('')) with points pt 7 ps 1.2 lc rgb '#FF0000' notitle, \
    'BAND_bands_with_arrows.dat' using 1:($5 + ($6-$5)/2):7 with labels offset 3,0 font 'Arial,10' tc rgb '#FF0000' notitle

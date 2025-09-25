#!/usr/bin/gnuplot
# Auto-generated gnuplot script by AMCheck C++
# Generated for: BAND.dat

# Terminal setup for high-resolution PDF output
set terminal pdf enhanced color size 5,4 font 'Arial,12' linewidth 1.5
set output 'BAND_bands.pdf'
# Increase rendering resolution for better zooming
set samples 1000
set isosamples 100

# Plot settings
set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb "gray"
set arrow from 0.000,graph(0,0) to 0.000,graph(1,1) nohead ls 1 lt 1 lw 2 lc rgb "gray"
set arrow from 0.691,graph(0,0) to 0.691,graph(1,1) nohead ls 1 dt 2 lt 1 lw 2 lc rgb "gray"
set arrow from 1.382,graph(0,0) to 1.382,graph(1,1) nohead ls 1 dt 2 lt 1 lw 2 lc rgb "gray"
set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb "gray"
set xtics font "Arial-Bold,15"
set ytics font "Arial-Bold,15"
# Axes tics and labels
set xtics ("M" 0.000, "{/Symbol G}" 0.691 , "M'" 1.382 ,) nomirror
set ylabel "E - E_{F} (eV)" font "Times-Bold,20" rotate by 90 
set label 2 "(High Sym KP)" at graph -0.45, graph 1.1 center norotate font ',35' tc rgb "black"
unset grid 
#set key right font 'Arial,10'
set border linewidth 1.5
#set tics font 'Arial,10'
set ylabel offset -3
set lmargin 12

# Set range for axes
#set xrange[0.0:1.39]
# Find y range dynamically
min_energy = 1e10
max_energy = -1e10
stats 'BAND_bands_with_arrows.dat' using 2 nooutput
min_energy = STATS_min
stats 'BAND_bands_with_arrows.dat' using 3 nooutput
if (STATS_min < min_energy) min_energy = STATS_min
stats 'BAND_bands_with_arrows.dat' using 2 nooutput
max_energy = STATS_max
stats 'BAND_bands_with_arrows.dat' using 3 nooutput
if (STATS_max > max_energy) max_energy = STATS_max
margin = (max_energy - min_energy) * 0.05
set yrange [-1:1]

# Plot settings
set zeroaxis ls 1.5 dt 2 lw 2.5 lc rgb "gray"
# Define arrow styles for band splitting
set style line 1 lc rgb 'blue' lt 1 lw 4.0
# Define styles for the vertical lines and end points
set style line 2 lc rgb '#FF0000' lt 1 lw 4.0 pt 7 ps 1.0
# Show vertical lines more prominently
set pointsize 1.5

# Plot the data
plot \
    'BAND_bands_with_arrows.dat' using 1:2 with lines lc rgb 'red' lw 2.5 title 'Spin Up', \
    'BAND_bands_with_arrows.dat' using 1:3 with lines lc rgb 'black' lw 2.5 title 'Spin Down', \
    'BAND_bands_with_arrows.dat' using ( $1 ):( $5 ):( 0 ):( $6 - $5 ) with vectors nohead lc rgb 'blue' lw 1.5 title 'Max Splitting', \
    'BAND_bands_with_arrows.dat' using 1:5:(sprintf('')) with points pt 7 ps 0.5 lc rgb 'blue' notitle, \
    'BAND_bands_with_arrows.dat' using 1:6:(sprintf('')) with points pt 7 ps 0.5 lc rgb 'blue' notitle, \
    'BAND_bands_with_arrows.dat' using 1:($5 + ($6-$5)/2):7 with labels offset 6,0 font 'Arial,13' tc rgb 'blue' notitle

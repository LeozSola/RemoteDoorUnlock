#!/usr/bin/env python3

import os

# Define the directory to analyze
directory = "~/CS410/PS3"

# Run my_histogram.py to generate histogram data
my_histogram_output = os.popen(f"python3 my_histogram.py {directory}").read()

# Create a gnuplot script to generate the plot
gnuplot_script = f"""
set terminal jpeg
set output 'plot.jpeg'
set title 'Histogram of File Types'
set xlabel 'File Types'
set ylabel 'Frequency'
set style data histograms
set xtic rotate by -45
plot '-' using 2:xtic(1) with histograms
{my_histogram_output}
"""

# Save the gnuplot script to a file
with open("plot.jpeg", "w") as f:
    f.write(gnuplot_script)

# Execute gnuplot to generate the plot
os.system("gnuplot plot.jpeg")

# Print the HTTP headers
print("Content-type: image/jpeg\n")

# Print the generated plot
with open("plot.jpeg", "rb") as f:
    print(f.read())

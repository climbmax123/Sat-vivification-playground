#!/bin/bash

# Parameters
logFile=$1
url=$2
output_folder=$3

# Create a temporary directory
temp_dir=$(mktemp -d)

# Download the file
wget --content-disposition -P "$temp_dir" "$url"

# Extract the filename
xz_file=$(find "$temp_dir" -name "*.cnf.xz")
cnf_file="${xz_file%.xz}"

# Extract the .cnf file
xz -d "$xz_file"

# Startzeit messen
start_time=$(date +%s.%N)

# Programm ausführen und Fehler abfangen
if ./sat-preprocessor -p "$cnf_file" "$output_folder/$(basename "$cnf_file")"; then
    # Endzeit messen und Laufzeit berechnen
    end_time=$(date +%s.%N)
    runtime=$(echo "$end_time - $start_time" | bc)
    # Laufzeit in CSV-Datei speichern
    echo "${cnf_file}, ${runtime}" >> "$logFile"
else
    echo "Fehler beim Verarbeiten der Datei $cnf_file" >> "$logFile"
    echo "${cnf_file}, ERROR" >> "$logFile"
fi

# Cleanup: Remove the temporary directory and all its contents
rm -rf "$temp_dir"
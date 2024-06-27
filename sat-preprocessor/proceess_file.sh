#!/bin/bash

# Parameter
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

# Function to count clauses and literals
count_clauses_literals() {
    local cnf_file=$1
    local clause_count=0
    local literal_count=0
    local total_literals=0

    while read -r line; do
        # Skip comments and problem line
        if [[ $line =~ ^c ]] || [[ $line =~ ^p ]]; then
            continue
        fi

        # Count clauses and literals
        if [[ -n $line ]]; then
            ((clause_count++))
            literals_in_clause=$(echo "$line" | awk '{print NF-1}')
            ((literal_count += literals_in_clause))
            ((total_literals += literals_in_clause))
        fi
    done < "$cnf_file"

    avg_literals_per_clause=$(echo "scale=2; $total_literals / $clause_count" | bc)
    echo "$clause_count $literal_count $avg_literals_per_clause"
}

# Count clauses and literals
read -r clause_count literal_count avg_literals_per_clause <<< $(count_clauses_literals "$cnf_file")

# Startzeit messen für sat-preprocessor
start_time=$(date +%s.%N)

# Programm ausführen und Fehler abfangen
if ./sat-preprocessor -p "$cnf_file" "$cnf_file"; then
    # Endzeit messen und Laufzeit berechnen
    end_time=$(date +%s.%N)
    sat_preprocessor_runtime=$(echo "$end_time - $start_time" | bc)
else
    sat_preprocessor_runtime=-1
    echo "Fehler beim Verarbeiten der Datei $cnf_file mit sat-preprocessor" >> "$logFile"
fi

# Startzeit messen für minisat
start_time=$(date +%s.%N)

# Programm ausführen und Fehler abfangen
timeout 60 ./minisat "$cnf_file"
minisat_exit_code=$?
if [[ $minisat_exit_code -eq 10 || $minisat_exit_code -eq 20 ]]; then
# Endzeit messen und Laufzeit berechnen
    end_time=$(date +%s.%N)
    minisat_runtime=$(echo "$end_time - $start_time" | bc)
else
    minisat_runtime=-1
    echo "Timout bei der Datei $cnf_file mit minisat (Timeout)" >> "$logFile"
fi

# Laufzeit und Metriken in CSV-Datei speichern
echo "${cnf_file}, ${sat_preprocessor_runtime}, ${clause_count}, ${literal_count}, ${avg_literals_per_clause}, ${minisat_runtime}" >> "$logFile"

# Cleanup: Remove the temporary directory and all its contents
rm -rf "$temp_dir"/*cnf
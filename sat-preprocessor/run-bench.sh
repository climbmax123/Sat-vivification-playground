#!/bin/bash

input_folder="/Users/christofer.held/Documents/Uni/BachelorArbeit/cnf-val-2022/"
output_folder="../testcases2"

# Erstellen des Ausgabeordners, falls nicht vorhanden
mkdir -p "$output_folder"

# Datei für die Speicherung der Laufzeiten
timing_file="${output_folder}/timings.csv"
echo "filename, runtime" > "$timing_file"

# Exportiere alle Funktionen und Variablen für die Parallel-Umgebung
export input_folder
export output_folder
export timing_file

# Funktion, die die Verarbeitung durchführt und die Laufzeit misst
process_file() {
    file=$1
    base_name=$(basename "$file")
    output_file="${output_folder}/$base_name"

    # Startzeit messen
    start_time=$(date +%s.%N)

    # Programm ausführen
    ./sat-preprocessor -p "$file" "$output_file"

    # Endzeit messen und Laufzeit berechnen
    end_time=$(date +%s.%N)
    runtime=$(echo "$end_time - $start_time" | bc)

    # Laufzeit in CSV-Datei speichern
    echo "${base_name}, ${runtime}" >> "$timing_file"
}

export -f process_file

# Finde alle CNF-Dateien und übergebe sie an GNU parallel
find "$input_folder" -name "*.cnf" | parallel process_file

echo "Verarbeitung abgeschlossen. Die Laufzeiten befinden sich in '$timing_file'."

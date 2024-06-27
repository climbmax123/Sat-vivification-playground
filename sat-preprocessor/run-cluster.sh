#!/bin/bash

input_file="instances.txt"
output_folder="."
timing_file="${output_folder}/timings.csv"

# Ensure the output folder exists
mkdir -p "$output_folder"
rm "../$timing_file"
rm ../elog.txt
rm ../log.txt
# Create or clear the timing file
echo "filename, sat_preprocessor_runtime, sat_preprocessor_clause_count, sat_preprocessor_literal_count, sat_preprocessor_avg_literals_per_clause, minisat_runtime" > "../$timing_file"

# Export variables for use in the cluster environment
export output_folder
export timing_file

# Iterate through each URL in the input file
while IFS= read -r url
do
    logName="${timing_file}"

    # Submit the job to the cluster
    qsub -N processFile -l h_vmem=2G -l bc1 -r n -e elog.txt -o log.txt process_file.sh $logName "$url" "$output_folder"
done < "$input_file"

echo "Verarbeitung abgeschlossen. Die Laufzeiten befinden sich in '$timing_file'."

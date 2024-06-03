#!/bin/bash

input_file="instances.txt"
output_folder="../testcases2"
timing_file="${output_folder}/timings.csv"

# Ensure the output folder exists
mkdir -p "$output_folder"

# Create or clear the timing file
echo "filename, runtime" > "$timing_file"

# Export variables for use in the cluster environment
export output_folder
export timing_file

# Iterate through each URL in the input file
while IFS= read -r url
do
    logName="${timing_file}"

    # Submit the job to the cluster
    qsub -N processFile -l h_vmem=2G -t 1-1 -r y -e /dev/null -o /dev/null path-to-script/process_file.sh $logName "$url" "$output_folder"
done < "$input_file"

echo "Verarbeitung abgeschlossen. Die Laufzeiten befinden sich in '$timing_file'."
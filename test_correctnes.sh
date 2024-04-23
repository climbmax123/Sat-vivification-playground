#!/bin/bash

# Navigate to the miniQu-with-preprocessing directory and run make
cd miniQu-with-preprocessing/
make
cd ..

# Navigate to the miniQu-without-preprocessing directory and run make
cd miniQu-without-preprocessing/
make
cd ..

# Start the loop
q=0
while [ $q -eq 0 ]
do
    # Generate input.qdimacs file
    python3 qbfuzz.py -v 40 -c 70 -o -r 0.5 --min=4 --max=7 -s 15 -o input.qdimacs

    # Transform it to output.qcir
    python3 qcir-conv.py input.qdimacs -o output.qcir

    # Run miniQU with preprocessing
    start_time1=$(date +%s.%N)
    ./miniQu-with-preprocessing/build/release/bin/miniQU output.qcir
    ret_code1=$?
    end_time1=$(date +%s.%N)
    time1=$(echo "$end_time1 - $start_time1" | bc)

    # Run miniQU without preprocessing
    start_time2=$(date +%s.%N)
    ./miniQu-without-preprocessing/build/release/bin/miniQU input.qdimacs
    ret_code2=$?
    end_time2=$(date +%s.%N)
    time2=$(echo "$end_time2 - $start_time2" | bc)

    # Compare return codes
    if [ $ret_code1 -ne $ret_code2 ]; then
        echo "Error: Return codes differ."
        echo "miniQu-with-preprocessing returned $ret_code1, while miniQu-without-preprocessing returned $ret_code2."
        break
    else
        echo "Success: Times: - With Preprocessing: $time1 s, Without: $time2 s."
    fi
done
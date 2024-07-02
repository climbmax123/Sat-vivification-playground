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
    python3 qbfuzz.py -v 10 -c 10 -o -r 0.5 -s 7 -o input.qdimacs

    # Transform it to output.qcir
    ./cmake-build-release-gcc-compiler/preprocess input.qdimacs output.qdimacs > /dev/null 2>&1
    # Run miniQU with preprocessing
    start_time1=$(date +%s)
    ./miniQu-without-preprocessing/build/release/bin/miniQU output.qdimacs > /dev/null 2>&1
    ret_code1=$?
    end_time1=$(date +%s)
    time1=$(($end_time1 - $start_time1))

    # Run miniQU without preprocessing
    start_time2=$(date +%s)
    ./miniQu-without-preprocessing/build/release/bin/miniQU input.qdimacs > /dev/null 2>&1
    ret_code2=$?
    end_time2=$(date +%s)
    time2=$(($end_time2 - $start_time2))

    # Compare return codes
    if [ $ret_code1 -ne $ret_code2 ]; then
        echo "Error: Return codes differ."
        echo "miniQu-with-preprocessing returned $ret_code1, while miniQu-without-preprocessing returned $ret_code2."
        break
    else
        echo "Success: Times: - With Preprocessing: ${time1} s, Without: ${time2} s."
    fi
done
cmake_minimum_required(VERSION 3.27)
project(Vivifcation)

set(CMAKE_CXX_STANDARD 23)

# Find OpenMP package
find_package(OpenMP REQUIRED)
find_package(TBB REQUIRED)

# Add -march=native flag and -mavx2 for GCC and Clang
#if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#        set(NATIVE_FLAGS "-march=native -mavx2")
#elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#        # MSVC does not support -march=native or -mavx2 directly
#        set(NATIVE_FLAGS "/arch:AVX2 -march=native")
#else()
#        message(FATAL_ERROR "Unsupported compiler")
#endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 ${NATIVE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 ${OpenMP_CXX_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")


add_executable(Vivifcation sat-solving/main.cpp
        sat-solving/CnfTester.cpp
        sat-solving/CnfTester.h
        sat-solving/preprocessing.cpp
        sat-solving/preprocessing.h
        sat-solving/types.h
        sat-solving/fastpreprocessing.cpp
        sat-solving/fastpreprocessing.h
        sat-solving/watchedpreprocessing.h
        sat-solving/watchedpreprocessing.cpp
        sat-solving/watchedliteralspreprocessing.cpp
        sat-solving/watchedliteralspreprocessing.h
        sat-solving/CSVWriter.cpp
        sat-solving/sortedwatchedliteralpreprocessing.cpp
        sat-solving/sortedwatchedliteralpreprocessing.h
        sat-solving/sortedwatchedliteralpreprocessing2.cpp
        sat-solving/sortedwatchedliteralpreprocessing2.h
        sat-solving/combinedpreprocessing.h
        sat-solving/combinedpreprocessing.cpp
)

add_executable(FasterCNF FasterCnfProcessing/main.cpp
        sat-preprocessor/main.cpp
        sat-preprocessor/combinedpreprocessing.cpp
        sat-preprocessor/combinedpreprocessing.h
)



add_executable(qbfuzz test-qbfuzz.cpp)

add_executable(preprocess qbf-preprocessing/main.cpp
        qbf-preprocessing/input-parser.cpp
        qbf-preprocessing/input-parser.h
        qbf-preprocessing/vivify.cpp
        qbf-preprocessing/vivify.h
        qbf-preprocessing/types.h
        qbf-preprocessing/unit_propagation.cpp
        qbf-preprocessing/unit_propagation.h
        qbf-preprocessing/pure_propagation.cpp
        qbf-preprocessing/pure_propagation.h
        qbf-preprocessing/sat_vivify.cpp
        qbf-preprocessing/sat_vivify.h
)

add_executable(sat-preprocessor sat-preprocessor/main.cpp sat-preprocessor/combinedpreprocessing.cpp sat-preprocessor/combinedpreprocessing.h sat-preprocessor/sortedwatchedliteralpreprocessing.cpp
        sat-preprocessor/sortedwatchedliteralpreprocessing.h sat-preprocessor/types.h sat-preprocessor/watchedliteralspreprocessing.h sat-preprocessor/watchedliteralspreprocessing.cpp
)

target_link_libraries(Vivifcation PRIVATE OpenMP::OpenMP_CXX)
target_link_libraries(FasterCNF PRIVATE TBB::tbb)

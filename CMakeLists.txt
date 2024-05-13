cmake_minimum_required(VERSION 3.27)
project(Vivifcation)

set(CMAKE_CXX_STANDARD 23)

# Find OpenMP package
find_package(OpenMP REQUIRED)


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
)

add_executable(qbfuzz test-qbfuzz.cpp)

add_executable(preprocess qbf-preprocessing/main.cpp
        qbf-preprocessing/input-parser.cpp
        qbf-preprocessing/input-parser.h
        qbf-preprocessing/vivify.cpp
        qbf-preprocessing/vivify.h
        qbf-preprocessing/types.h)

target_link_libraries(Vivifcation PRIVATE OpenMP::OpenMP_CXX)


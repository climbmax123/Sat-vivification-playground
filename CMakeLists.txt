cmake_minimum_required(VERSION 3.27)
project(Vivifcation)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")

file(GLOB_RECURSE CADICAL_FILES cadical/*.cpp cadical/*.hpp)


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
)

add_executable(qbfuzz test-qbfuzz.cpp)

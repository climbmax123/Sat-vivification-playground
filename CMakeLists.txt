cmake_minimum_required(VERSION 3.27)
project(Vivifcation)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE CADICAL_FILES cadical/*.cpp cadical/*.hpp)


add_executable(Vivifcation main.cpp
        CnfTester.cpp
        CnfTester.h
        preprocessing.cpp
        preprocessing.h
        types.h
        Preprocessor.cpp
        Preprocessor.h
        )

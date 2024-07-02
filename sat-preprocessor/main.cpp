//
// Created by Christofer Held on 02.06.24.
//


#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

#include <getopt.h>
#include <unordered_map>
#include "types.h"
#include "sortedwatchedliteralpreprocessing.h"
#include "watchedliteralspreprocessing.h"
#include "combinedpreprocessing.h"




// Parse the cnf file

CDNF_formula parseCDNFFromFile(std::string path) {
    CDNF_formula cnf;
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line);
            std::vector<int> clause;
            int literal;
            while (iss >> literal) {
                if (literal == 0) break;
                clause.push_back(literal);
            }
            if (!clause.empty()) {
                cnf.push_back(clause);
            }
        }
        file.close();
        std::cout << "CNF loaded from: " << path << std::endl;
    } else {
        std::cerr << "Unable to open file for reading: " << path << std::endl;
    }
    return std::move(cnf);
}


void writeCDNFFile(std::string path, CDNF_formula &cnf) {
    std::ofstream file(path);
    int max = 0;
    for(auto & c: cnf){
        for(auto l: c){
            if(std::abs(l) > max){
                max = std::abs(l);
            }
        }
    }

    file << "p cnf " << max << " " << cnf.size() << "\n";
    if (file.is_open()) {
        for (const auto &clause: cnf) {
            for (const auto &literal: clause) {
                file << literal << " ";
            }
            file << "0\n"; // CNF-Ende einer Klausel
        }
        file.close();
        std::cout << "CNF saved to: " << path << std::endl;
    } else {
        std::cerr << "Unable to open file for writing: " << std::endl;
    }

}


int main(int argc, char *argv[]) {
    int opt;
    enum {
        PURE, UNIT, VIVIFY, COMBIEND, SORTEDVIVIFY
    } mode;

    const char *options = "u:v:p:c:s";
    while ((opt = getopt(argc, argv, options)) != -1) {
        switch (opt) {
            case 'u':
                mode = UNIT;
                break;
            case 'v':
                mode = VIVIFY;
                break;
            case 'p':
                mode = PURE;
                break;
            case 'c':
                mode = COMBIEND;
                break;
            case 's':
                mode = SORTEDVIVIFY;
                break;
            default:
                std::cerr << "Invalid option provided." << std::endl;
                return 1;
        }
    }

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [-u|-v|-p|-c|-s] <inputfile> <outputfile>" << std::endl;
        return 1;
    }

    std::cout << "Loading the CNF formula" << std::endl;
    auto start_load = std::chrono::high_resolution_clock::now();
    CDNF_formula cnf = parseCDNFFromFile(argv[2]);
    auto end_load = std::chrono::high_resolution_clock::now();
    auto duration_load = duration_cast<std::chrono::milliseconds>(end_load - start_load).count();
    std::cout << "Loaded CNF with " << cnf.size() << " clauses in " << duration_load << " ms" << std::endl;


    auto start_process = std::chrono::high_resolution_clock::now();
    int timeLimitInSeconds = 60*60; // we give it a timeout of 1h
    std::unordered_map<int, int> mapping;

    switch (mode) {
        case UNIT:
            std::cout << "Start Unit Propagation" << std::endl;
            watched_literals::unit_propagation(cnf, timeLimitInSeconds);
            break;
        case VIVIFY:
            std::cout << "Start Vivify" << std::endl;
            watched_literals::vivify(cnf, timeLimitInSeconds);
            break;
        case PURE:
            std::cout << "Start Pure elimination" << std::endl;
            mapping = combined::creat_mapping(cnf);
            combined::pureLiteralElimination(cnf, mapping);
            break;
        case COMBIEND:
            std::cout << "Start Combined elimination" << std::endl;
            watched_literals::unit_propagation(cnf, timeLimitInSeconds);
            mapping = combined::creat_mapping(cnf);
            combined::pureLiteralElimination(cnf, mapping);
            break;
        case SORTEDVIVIFY:
            std::cout << "Start Sorted Vivify with Pure elimination" << std::endl;
            sorted::vivify_with_sorted_pure_lit(cnf, timeLimitInSeconds);
            break;
    }

    auto end_process = std::chrono::high_resolution_clock::now();
    auto duration_process = duration_cast<std::chrono::milliseconds>(end_process - start_process).count();
    std::cout << "Finished preprocessing in " << duration_process << "ms" << std::endl;

    // Process the parsed QBF...
    std::cout << "Writing result to " << argv[3] << std::endl;
    writeCDNFFile(argv[3], cnf);
    std::cout << "Finished writing file program exist now" << std::endl;
    return 0;


}
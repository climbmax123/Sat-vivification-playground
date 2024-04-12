//
// Created by Christofer Held on 11.04.24.
//

#ifndef VIVIFCATION_QBFTESTER_H
#define VIVIFCATION_QBFTESTER_H

#include "types.h"

#include <iostream>
#include <vector>
#include <set>
#include <random>
#include <ctime>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <sstream>


class QBFTester {
private:
    int num_variables;
    int num_clauses;
    CDNF_formula original_clauses;
    std::vector<int> existential_quantified;
    std::vector<int> universal_quantified;
    std::mt19937 rng;
    std::vector<int> quantifiers;

    void saveQBFToFile(const std::string &filePath, const CDNF_formula &clauses, const CDNF_formula &terms);

    bool callQbfSolver(const std::string &qbfFilePath, const std::string &resultFilePath);

    std::string exec(const char* cmd);

    int countVariables(const CDNF_formula& clauses);

public:
    QBFTester(int variables, int clauses);

    std::pair<CDNF_formula, CDNF_formula> generateQBF();

    bool testQBF(const CDNF_formula &adapted_clauses, const CDNF_formula &adapted_terms);

    void printOriginalQBF();

    static void printQBF(const CDNF_formula &clauses, const CDNF_formula &terms);

    void measureDifference(const CDNF_formula &clauses, const CDNF_formula &terms);

    std::pair<CDNF_formula, CDNF_formula> parseQDIMACS(const std::string& qdimacs);
};


#endif //VIVIFCATION_QBFTESTER_H

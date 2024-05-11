#include <iostream>
#include "CnfTester.h"
#include "../Preprocessor.h"
#include "preprocessing.h"
#include "fastpreprocessing.h"
#include "watchedpreprocessing.h"
#include "watchedliteralspreprocessing.h"
#include "CSVWriter.h"
#include <chrono>

int numLiterals(const CDNF_formula &cnf) {
    std::set<int> literals;
    for (const auto &clause: cnf) {
        for (const auto &lit: clause) {
            literals.insert(std::abs(lit));
        }
    }
    return literals.size();
}

bool equals(CDNF_formula &s1, CDNF_formula &s2) {
    if (s1.size() != s2.size()) return false;
    for (int i = 0; i < s1.size(); i++) {
        if (s1[i].size() != s2[i].size()) return false;
        for (int j = 0; j < s1[i].size(); j++) {
            if (s1[i][j] != s2[i][j]) return false;
        }
    }
    return true;
}


int main() {

    CSVWriter writer("runtime_statistics3.csv");

    for (int vars = 1; vars <= 16 ; vars *= 4) {
        for(int clauses = 10; clauses <= 1000; clauses *= 10) {
            CNFTester tester( vars*clauses, clauses);
            for (int test = 0; test < 10000; test++) {
                auto cnf = tester.generateCNF();
                // normal unit prop
                CDNF_formula unit_prop = normal::unit_propagation(cnf);
                // normal pure literal elimination
                CDNF_formula pure_literal = normal::pureLiteralElimination(cnf);
                if (!equals(cnf, pure_literal)) {
                    std::cout << "vars: \t " << vars << "\tclauses: \t " << clauses << "\t test: \t " << test << std::endl;
                }
                // combination
                CDNF_formula comb = normal::pureLiteralElimination(cnf);
                CDNF_formula new_comb = normal::unit_propagation(comb);
                while (!equals(comb, new_comb) && !cnf.empty() && !cnf[0].empty()) {
                    comb = new_comb;
                    new_comb = normal::unit_propagation(normal::pureLiteralElimination(comb));
                }
                writer.writeData(cnf.size(), numLiterals(cnf), unit_prop.size(), numLiterals(unit_prop),
                                 pure_literal.size(),
                                 numLiterals(pure_literal), new_comb.size(), numLiterals(new_comb));

            }
        }

    }

    return 0;
}

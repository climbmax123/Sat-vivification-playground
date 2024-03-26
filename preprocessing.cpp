//
// Created by Christofer Held on 07.03.24.
//

#include "preprocessing.h"

#include <set>
#include <vector>
#include <algorithm>
#include <iostream>

void printCNF(const CDNF_formula &cnf) {
    for (const auto &clause: cnf) {
        std::cout << "(";
        for (const auto &literal: clause) {
            std::cout << literal << " ";
        }
        std::cout << ") ";
    }
    std::cout << std::endl;
}

CDNF_formula unit_propagation(CDNF_formula cnf) {
    bool progress = true;
    std::vector<std::pair<int, int>> found_first_units;

    while (progress) {
        progress = false;
        std::set<int> unit_clauses_set;
        std::vector<int> unit_clauses;

        // Find all unit clauses and check for contradictions
        for (const auto &clause: cnf) {
            if (clause.size() == 1) {
                int unit = clause.front();
                if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                    return {{}};
                }
                if (unit_clauses_set.insert(unit).second) {
                    unit_clauses.push_back(unit);
                }
            }
        }

        if (unit_clauses.empty()) {
            break;
        }

        for (int unit: unit_clauses) {
            // Remove clauses that are satisfied by the unit clause
            cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                                     [unit](const std::vector<int> &clause) {
                                         return std::find(clause.begin(), clause.end(), unit) != clause.end();
                                     }), cnf.end());

            // Remove the negation of the unit clause from all clauses, indicating a contradiction if a clause becomes empty
            for (auto &clause: cnf) {
                clause.erase(std::remove(clause.begin(), clause.end(), -unit), clause.end());
                if (clause.empty()) {
                    // A contradiction found, return an empty formula
                    return {{}};
                }
            }

            progress = true;
        }
    }
    return cnf;
}

// for now, I take any literal. (in that case the next)
int select_a_literal(const std::vector<int> &c, const std::vector<int> &cb) {
    for (int lit: c) {
        if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
            return lit;
        }
    }
    return 0;
}


CDNF_formula reduced1_vivify(CDNF_formula cnf) {
    //std::cout << "Startclause: \t";
    //printCNF(cnf);
    cnf = unit_propagation(cnf);
    //std::cout << "UP:  \t\t\t";
    //printCNF(cnf);

    bool change = true;
    CDNF_formula new_cnf;

    while (change) {
        change = false;
        int size = cnf.size();

        for (int i = 0; i < size; i++) {
            std::vector<int> c = cnf[i];
            std::cout << "Took clause: \t";
            printCNF({c});

            cnf.erase(cnf.begin() + i);

            CDNF_formula cnf_backup = cnf;

            std::vector<int> cb; // the reduced clause to write back

            bool shortened = false;

            while (!shortened && c != cb) {

                int l = select_a_literal(c, cb);

                // Pseudocode: c_b ← c_b ∪ {l}; Σ_b ← (Σ_b ∪ {¬l});
                cb.push_back(l);
                cnf_backup.push_back({-l});

                std::cout << "lit \t" << l << ": \t";
                printCNF(cnf_backup);

                cnf_backup = unit_propagation(cnf_backup);

                std::cout << "UP:  \t\t\t";
                printCNF(cnf_backup);

                if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                    cnf.insert(cnf.begin() + i, cb);
                    if( c != cb) {
                        shortened = true;
                        change = true;
                    }
                    std::cout << "New Clause:  \t";
                    printCNF(cnf);
                }

                if (cnf_backup.empty()){
                    break;
                }
            }
            if (!shortened) {
                cnf.insert(cnf.begin() + i, c);
            }
        }
    }
    return cnf;
}


// TODO: TO FIX VIFIVCATION:
/**
 * Step1: Implement UP so that it does 2 things:
 * First Check if False
 * Secondly give back a list of all found Unit Clauses
*/

CDNF_formula UP(CDNF_formula cnf, std::vector<std::pair<int,int>> & found_units){
    bool progress = true;
    int step_counter = 0;
    std::vector<std::pair<int, int>> found_first_units;

    while (progress) {
        progress = false;
        std::set<int> unit_clauses_set;
        std::vector<int> unit_clauses;

        // Find all unit clauses and check for contradictions
        for (const auto &clause: cnf) {
            if (clause.size() == 1) {
                int unit = clause.front();
                if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                    return {{}};
                }
                found_units.emplace_back(step_counter, unit);
                if (unit_clauses_set.insert(unit).second) {
                    unit_clauses.push_back(unit);
                }
            }
        }

        if (unit_clauses.empty()) {
            break;
        }

        for (int unit: unit_clauses) {
            // Remove clauses that are satisfied by the unit clause
            cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                                     [unit](const std::vector<int> &clause) {
                                         return std::find(clause.begin(), clause.end(), unit) != clause.end();
                                     }), cnf.end());

            // Remove the negation of the unit clause from all clauses, indicating a contradiction if a clause becomes empty
            for (auto &clause: cnf) {
                clause.erase(std::remove(clause.begin(), clause.end(), -unit), clause.end());
                if (clause.empty()) {
                    // A contradiction found, return an empty formula
                    return {{}};
                }
            }

            progress = true;
        }
        step_counter++;
    }

    return cnf;
}

CDNF_formula vivify(CDNF_formula cnf){
    // apply the first unit propagation
    cnf = unit_propagation(cnf);

    bool change = true;

    // Pseudocode: while change do
    while (change) {
        // Pseudocode: change ← false;
        change = false;

        // Pseudocode: foreach c ∈ Σ do
        for (int i = 0; i < cnf.size(); i++){

            std::vector<int> c = cnf[i];
            cnf.erase(cnf.begin() + i);
            CDNF_formula cnf_backup = cnf;

            std::vector<int> cb;
            bool shortened = false;

            while (!shortened && c != cb) {
                int l = select_a_literal(c, cb);
                cb.push_back(l);
                cnf_backup.push_back({-l});

                std::vector<std::pair<int,int>> found_unit_clauses;
                cnf_backup = UP(cnf, found_unit_clauses);

                if (cnf_backup.empty()){
                    break; // found a sat solution
                }

                // case 1 : UP() is False
                if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                    cnf.insert(cnf.begin() + i, cb);
                    if(c != cb) {
                        shortened = true;
                        change = true;
                    }
                } // case 2  : has found some additional unit clauses
                else if (found_unit_clauses.size() > 1) {
                    // case: posive
                    if (found_unit_clauses[1].second > 0){
                        if(cb.size() + 1 < c.size()){
                            cb.push_back(found_unit_clauses[1].second);
                            cnf.insert(cnf.begin() + i, cb);
                            shortened = true;
                        }
                    }
                    // case: negative
                    else {
                        std::vector<int> new_clause;
                        for (int l: c) {
                            if (l != found_unit_clauses[1].second) {
                                new_clause.push_back(l);
                            }
                        }
                        cnf.insert(cnf.begin() + i, new_clause);
                        shortened = true;
                    }
                }
            }
            // Pseudocode: if Not(shortened) then Σ ← Σ ∪ {c};
            if (!shortened) {
                cnf.insert(cnf.begin() + i, c); // Add back the unshortened clause
            } else {
                // Pseudocode: else change ← true;
                change = true;
            }
        }
    }
    return cnf;
}



/**
 * Step2: Implement the second phase
 * if postive lit add new clause with pos lit
 * if multiple neg lit remove lits that where neg
 *
*/
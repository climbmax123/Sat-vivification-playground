//
// Created by Christofer Held on 07.03.24.
//

#include "preprocessing.h"

#include <set>
#include <vector>
#include <algorithm>

CDNF_formula unit_propagation(CDNF_formula cnf) {
    bool progress = true;

    while (progress) {
        progress = false;
        std::set<int> unit_clauses_set;
        std::vector<int> unit_clauses;

        // Find all unit clauses and check for contradictions
        for (const auto &clause: cnf) {
            if (clause.size() == 1) {
                int unit = clause.front();
                if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                    return {};
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
                    return {};
                }
            }

            progress = true;
        }
    }

    return cnf;
}


CDNF_formula advanced_unit_propagation(CDNF_formula cnf) {
    bool progress = true;

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


bool unit_propagation_with_literal(CDNF_formula cnf, int literal) {
    // Add the literal as a unit clause to the CNF
    cnf.push_back({literal});

    // Apply unit propagation
    CDNF_formula result = unit_propagation(cnf);

    // If the result is empty, it means a contradiction was found
    if (result.empty()) {
        return true; // A contradiction implies the literal led to a unit propagation conflict
    }

    // Otherwise, no contradiction was found
    return false;
}

std::vector<int> conflict_analyze_and_learn(const std::vector<int> &conflict_clause, const CDNF_formula &cnf) {
    std::set<int> learned_literals;

    // For simplicity, we just negate all literals in the conflict clause.
    for (int literal: conflict_clause) {
        learned_literals.insert(-literal);
    }

    // Simplistic approach: return a clause consisting of all negated literals that led to the conflict.
    return std::vector<int>(learned_literals.begin(), learned_literals.end());
}

// For now, we just select any literral
int select_a_literal(const std::vector<int> &c, const std::vector<int> &cb) {
    for (int lit: c) {
        if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
            return lit;
        }
    }
    return 0;
}

CDNF_formula vivify(CDNF_formula cnf) {
    // Pseudocode: change ← true;
    bool change = true;

    // Pseudocode: while change do
    while (change) {
        // Pseudocode: change ← false;
        change = false;

        // Pseudocode: foreach c ∈ Σ do
        for (auto it = cnf.begin(); it != cnf.end(); ++it) {
            std::vector<int> c = *it;
            // Pseudocode: Σ ← Σ \ {c}; Σ_b ← Σ;
            cnf.erase(it); // Remove current clause for processing
            CDNF_formula cnf_backup = cnf;

            // Pseudocode: c_b ← ∅; shortened ← false;
            std::vector<int> cb;
            bool shortened = false;

            // Pseudocode: while (Not(shortened) And (c ≠ c_b)) do
            while (!shortened && c != cb) {
                // Pseudocode: l ← select_a_literal(c \ c_b);
                int l = select_a_literal(c, cb);

                // Pseudocode: c_b ← c_b ∪ {l}; Σ_b ← (Σ_b ∪ {¬l});
                cb.push_back(l);
                cnf_backup.push_back({-l});

                // Pseudocode: if ⊥ ∈ UP(Σb)  then
                if (unit_propagation(cnf_backup).empty()) {
                    // Pseudocode: c_l ← conflict_analyze_and_learn();
                    std::vector<int> conflicting_clause = cnf_backup.back();
                    std::vector<int> cl = conflict_analyze_and_learn(conflicting_clause, cnf_backup);


                    // Pseudocode: if c_l ⊂ c then
                    if (std::includes(c.begin(), c.end(), cl.begin(), cl.end())) {
                        //Pseudocode: Σ ← Σ ∪ {c_l};
                        cnf.push_back(cl);
                        shortened = true;
                    } else {
                        //Pseudocode: if |c_l| < |c| then Σ ← Σ ∪ {c_l}; c_b ← c;
                        if (cl.size() < it->size()) {
                            *it = cl; // Replace with shortened clause
                            cb = c;
                        }
                        //Pseudocode: if c ≠ c_b then Σ ← Σ ∪ {c_b};
                        if (c != cb) {
                            cnf.push_back(cb);
                            shortened = true;
                        }
                    }
                } else {
                    // Pseudocode: if ∃(l_s ∈ (c \ c_b)) s.t. l_s ∈ UP(Σ_b) then
                    for (int l_s: c) {
                        if (std::find(cb.begin(), cb.end(), l_s) == cb.end() && // l_s ∈ (c \ c_b)
                            unit_propagation_with_literal(cnf_backup, l_s)) { // l_s ∈ UP(Σ_b) TODO: FIX

                            // Pseudocode: if (c \ c_b) ≠ {l_s} then
                            if (!(c.size() == 1 && c[0] == l_s)) {
                                // Pseudocode: Σ ← Σ ∪ {c_b ∪ {l_s}};
                                std::vector<int> new_clause(cb.begin(), cb.end());
                                new_clause.push_back(l_s);
                                cnf.push_back(new_clause);
                                shortened = true; // Mark as shortened
                            }
                        }
                    }

                    // Pseudocode: if ∃(l_s ∈ (c \ c_b)) s.t. ¬l_s ∈ UP(Σ_b) then
                    for (int l_s: c) {
                        if (std::find(cb.begin(), cb.end(), l_s) == cb.end() && // l_s ∈ (c \ c_b)
                            unit_propagation_with_literal(cnf_backup, -l_s)) { // ¬l_s ∈ UP(Σ_b)

                            // Pseudocode: Σ ← Σ ∪ {c \ {l_s}};
                            std::vector<int> new_clause;
                            for (int l: c) {
                                if (l != l_s) {
                                    new_clause.push_back(l);
                                }
                            }
                            cnf.push_back(new_clause);
                            shortened = true; // Mark as shortened
                        }
                    }
                }

                // Pseudocode: if Not(shortened) then Σ ← Σ ∪ {c};
                if (!shortened) {
                    cnf.push_back(c); // Add back the unshortened clause
                } else {
                    // Pseudocode: else change ← true;
                    change = true;
                }
            }
        }
    }
    // Pseudocode: return Σ;
    return cnf;
}


CDNF_formula vivify2(CDNF_formula cnf) {
    // Pseudocode: change ← true;
    bool change = true;

    // Pseudocode: while change do
    while (change) {
        // Pseudocode: change ← false;
        change = false;

        // Pseudocode: foreach c ∈ Σ do
        for (auto it = cnf.begin(); it != cnf.end(); ++it) {
            std::vector<int> c = *it;
            // Pseudocode: Σ ← Σ \ {c}; Σ_b ← Σ;
            cnf.erase(it); // Remove current clause for processing
            CDNF_formula cnf_backup = cnf;

            // Pseudocode: c_b ← ∅; shortened ← false;
            std::vector<int> cb;
            bool shortened = false;

            // Pseudocode: while (Not(shortened) And (c ≠ c_b)) do
            while (!shortened && c != cb) {
                // Pseudocode: l ← select_a_literal(c \ c_b);
                int l = select_a_literal(c, cb);

                // Pseudocode: c_b ← c_b ∪ {l}; Σ_b ← (Σ_b ∪ {¬l});
                cb.push_back(l);
                cnf_backup.push_back({-l});

                // Pseudocode: if ⊥ ∈ UP(Σb)  then
                if (unit_propagation(cnf_backup).empty()) {
                    // Pseudocode: c_l ← conflict_analyze_and_learn();
                    std::vector<int> conflicting_clause = cnf_backup.back();
                    std::vector<int> cl = conflict_analyze_and_learn(conflicting_clause, cnf_backup);


                    // Pseudocode: if c_l ⊂ c then
                    if (std::includes(c.begin(), c.end(), cl.begin(), cl.end())) {
                        //Pseudocode: Σ ← Σ ∪ {c_l};
                        cnf.push_back(cl);
                        shortened = true;
                    } else {
                        //Pseudocode: if |c_l| < |c| then Σ ← Σ ∪ {c_l}; c_b ← c;
                        if (cl.size() < it->size()) {
                            *it = cl; // Replace with shortened clause
                            cb = c;
                        }
                        //Pseudocode: if c ≠ c_b then Σ ← Σ ∪ {c_b};
                        if (c != cb) {
                            cnf.push_back(cb);
                            shortened = true;
                        }
                    }
                } else {
                    // Pseudocode: if ∃(l_s ∈ (c \ c_b)) s.t. l_s ∈ UP(Σ_b) then
                    for (int l_s: c) {
                        if (std::find(cb.begin(), cb.end(), l_s) == cb.end() && // l_s ∈ (c \ c_b)
                            unit_propagation_with_literal(cnf_backup, l_s)) { // l_s ∈ UP(Σ_b) TODO: FIX

                            // Pseudocode: if (c \ c_b) ≠ {l_s} then
                            if (!(c.size() == 1 && c[0] == l_s)) {
                                // Pseudocode: Σ ← Σ ∪ {c_b ∪ {l_s}};
                                std::vector<int> new_clause(cb.begin(), cb.end());
                                new_clause.push_back(l_s);
                                cnf.push_back(new_clause);
                                shortened = true; // Mark as shortened
                            }
                        }
                    }

                    // Pseudocode: if ∃(l_s ∈ (c \ c_b)) s.t. ¬l_s ∈ UP(Σ_b) then
                    for (int l_s: c) {
                        if (std::find(cb.begin(), cb.end(), l_s) == cb.end() && // l_s ∈ (c \ c_b)
                            unit_propagation_with_literal(cnf_backup, -l_s)) { // ¬l_s ∈ UP(Σ_b)

                            // Pseudocode: Σ ← Σ ∪ {c \ {l_s}};
                            std::vector<int> new_clause;
                            for (int l: c) {
                                if (l != l_s) {
                                    new_clause.push_back(l);
                                }
                            }
                            cnf.push_back(new_clause);
                            shortened = true; // Mark as shortened
                        }
                    }
                }

                // Pseudocode: if Not(shortened) then Σ ← Σ ∪ {c};
                if (!shortened) {
                    cnf.push_back(c); // Add back the unshortened clause
                } else {
                    // Pseudocode: else change ← true;
                    change = true;
                }
            }
        }
    }
    // Pseudocode: return Σ;
    return cnf;
}




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
            //std::cout << "Took clause: \t";
            //printCNF({c});

            cnf.erase(cnf.begin() + i);

            CDNF_formula cnf_backup = cnf;

            std::vector<int> cb; // the reduced clause to write back

            bool shortened = false;

            while (!shortened && c != cb) {

                int l = select_a_literal(c, cb);

                // Pseudocode: c_b ← c_b ∪ {l}; Σ_b ← (Σ_b ∪ {¬l});
                cb.push_back(l);
                cnf_backup.push_back({-l});

                //std::cout << "lit \t" << l << ": \t";
                //printCNF(cnf_backup);

                cnf_backup = advanced_unit_propagation(cnf_backup);

                //std::cout << "UP:  \t\t\t";
                //printCNF(cnf_backup);

                if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                    cnf.insert(cnf.begin() + i, cb);
                    if( c != cb) {
                        shortened = true;
                        change = true;
                    }
                    //std::cout << "New Clause:  \t";
                    //printCNF(cnf);
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


CDNF_formula reduced2_vivify(CDNF_formula cnf) {
    bool change = true;
    return cnf;
}


// Fall 1: UP findet eine lösung:
// Fall 2: UP findet eine contradiction
// Fall 3: UP findet nix


CDNF_formula UP_reduce(CDNF_formula cnf) {
    std::cout << "Startclause: \t";
    printCNF(cnf);
    cnf = unit_propagation(cnf);
    std::vector<int> c = cnf[0];
    cnf.erase(cnf.begin());
    std::cout << "Took clause: \t";
    printCNF({c});
    for (auto lit: c) {
        std::cout << "lit \t" << lit << ": \t";
        cnf.push_back({-lit});
        cnf = advanced_unit_propagation(cnf);
        if (!cnf.empty() && cnf.front().empty()) {
            std::cout << "empty";
        }
        printCNF(cnf);
    }
    return cnf;
}

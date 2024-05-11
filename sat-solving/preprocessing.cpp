//
// Created by Christofer Held on 07.03.24.
//

#include "preprocessing.h"

#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>

namespace normal {

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

    CDNF_formula UP(CDNF_formula cnf, std::vector<std::pair<int, int>> &found_units) {
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


    CDNF_formula vivify(CDNF_formula cnf) {
        bool change = true;

        while (change) {
            change = false;
            cnf = unit_propagation(cnf);
            for (int i = 0; i < cnf.size(); i++) {
                std::vector<int> c = cnf[i];
                cnf.erase(cnf.begin() + i);
                CDNF_formula cnf_backup = cnf;
                std::vector<int> cb;
                bool shortened = false;

                while (!shortened && c != cb) {
                    int l = select_a_literal(c, cb);
                    cb.push_back(l);
                    cnf_backup.push_back({-l});
                    std::vector<std::pair<int, int>> found_unit_clauses;
                    cnf_backup = UP(cnf_backup, found_unit_clauses);
                    if (cnf_backup.empty()) {
                        return {}; // found a sat solution
                    }
                    if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                        cnf.insert(cnf.begin() + i, cb);
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else if (found_unit_clauses.size() > 1) {
                        if (found_unit_clauses[1].second > 0) {
                            if (cb.size() + 1 < c.size()) {
                                cb.push_back(found_unit_clauses[1].second);
                                cnf.insert(cnf.begin() + i, cb);
                                shortened = true;
                            }
                        } else {
                            int k = 0;
                            for (int s = 1; s < found_unit_clauses.size(); s++) {
                                if (std::find(c.begin(), c.end(), -found_unit_clauses[s].second) != c.end()) {
                                    k = s;
                                    break;
                                }
                            }
                            if (k != 0) {
                                std::vector<int> new_clause;
                                for (int l: c) {
                                    if (l != -found_unit_clauses[k].second) {
                                        new_clause.push_back(l);
                                    }
                                }
                                cnf.insert(cnf.begin() + i, new_clause);
                                shortened = true;
                            }
                        }
                    }
                }
                if (!shortened) {
                    cnf.insert(cnf.begin() + i, c);
                } else {
                    change = true;
                }
            }
        }
        return cnf;
    }


    CDNF_formula pureLiteralElimination(CDNF_formula formula) {
        std::set<int> literals; // Track the polarity of each literal

        // Count the polarity of each literal in the formula
        for (const auto &clause: formula) {
            for (int literal: clause) {
               literals.insert(literal);
            }
        }

        // Remove clauses containing pure literals
        CDNF_formula newFormula;
        for (const auto &clause: formula) {
            bool clauseContainsPureLiteral = false;
            for (int literal: clause) {
                if (!literals.contains(-literal)){
                    clauseContainsPureLiteral = true;
                    break;
                }
            }
            if (!clauseContainsPureLiteral) {
                newFormula.push_back(clause);
            }
        }
        return newFormula;
    }


    CDNF_formula vivify_with_pure_lit(CDNF_formula cnf) {
        bool change = true;
        while (change) {
            change = false;
            cnf = unit_propagation(cnf);
            cnf = pureLiteralElimination(cnf);
            for (int i = 0; i < cnf.size(); i++) {
                std::vector<int> c = cnf[i];
                cnf.erase(cnf.begin() + i);
                CDNF_formula cnf_backup = cnf;
                std::vector<int> cb;
                bool shortened = false;

                while (!shortened && c != cb) {
                    int l = select_a_literal(c, cb);
                    cb.push_back(l);
                    cnf_backup.push_back({-l});
                    std::vector<std::pair<int, int>> found_unit_clauses;
                    cnf_backup = UP(cnf_backup, found_unit_clauses);
                    if (cnf_backup.empty()) {
                        return {};
                    }
                    if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                        cnf.insert(cnf.begin() + i, cb);
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else if (found_unit_clauses.size() > 1) {
                        if (found_unit_clauses[1].second > 0) {
                            if (cb.size() + 1 < c.size()) {
                                cb.push_back(found_unit_clauses[1].second);
                                cnf.insert(cnf.begin() + i, cb);
                                shortened = true;
                            }
                        } else {
                            int k = 0;
                            for (int s = 1; s < found_unit_clauses.size(); s++) {
                                if (std::find(c.begin(), c.end(), -found_unit_clauses[s].second) != c.end()) {
                                    k = s;
                                    break;
                                }
                            }
                            if (k != 0) {
                                std::vector<int> new_clause;
                                for (int l: c) {
                                    if (l != -found_unit_clauses[k].second) {
                                        new_clause.push_back(l);
                                    }
                                }
                                cnf.insert(cnf.begin() + i, new_clause);
                                shortened = true;
                            }
                        }
                    }
                    cnf_backup = pureLiteralElimination(cnf_backup);
                    if (cnf_backup.empty()) {
                        return {};
                    }
                }
                if (!shortened) {
                    cnf.insert(cnf.begin() + i, c);
                } else {
                    change = true;
                }
            }
        }
        return cnf;
    }

}
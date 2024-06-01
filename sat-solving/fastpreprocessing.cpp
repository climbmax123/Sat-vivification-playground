//
// Created by Christofer Held on 12.04.24.
//

#include "fastpreprocessing.h"

#include <set>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>

namespace fast {

    void UnitPropagation(CDNF_formula &cnf) {
        bool progress = true;
        while (progress) {
            progress = false;
            std::set<int> unit_clauses_set;
            std::vector<int> unit_clauses;
            for (const auto &clause: cnf) {
                if (clause.size() == 1) {
                    int unit = clause.front();
                    if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
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
                cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                                         [unit](const std::vector<int> &clause) {
                                             return std::find(clause.begin(), clause.end(), unit) != clause.end();
                                         }), cnf.end());

                for (auto &clause: cnf) {
                    clause.erase(std::remove(clause.begin(), clause.end(), -unit), clause.end());
                    if (clause.empty()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
                    }
                }
                progress = true;
            }
        }
    }


    void UP(CDNF_formula &cnf, std::vector<std::pair<int, int>> &found_units) {
        bool progress = true;
        int step_counter = 0;

        while (progress) {
            progress = false;
            std::set<int> unit_clauses_set;
            std::vector<int> unit_clauses;

            for (const auto &clause: cnf) {
                if (clause.size() == 1) {
                    int unit = clause.front();
                    if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
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
                cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                                         [unit](const std::vector<int> &clause) {
                                             return std::find(clause.begin(), clause.end(), unit) != clause.end();
                                         }), cnf.end());

                for (auto &clause: cnf) {
                    clause.erase(std::remove(clause.begin(), clause.end(), -unit), clause.end());
                    if (clause.empty()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
                    }
                }
                progress = true;
            }

            step_counter++;
        }
    }


    void PureLiteralElimination(CDNF_formula &formula) {
        std::unordered_map<int, int> literalCount;
        for (const auto &clause: formula) {
            for (int literal: clause) {
                int key = abs(literal);
                if (literalCount.find(key) == literalCount.end()) {
                    literalCount[key] = (literal > 0) ? 1 : -1;
                } else if ((literal > 0 && literalCount[key] < 0) || (literal < 0 && literalCount[key] > 0)) {
                    literalCount[key] = 0;
                }
            }
        }
        for (auto clauseIt = formula.begin(); clauseIt != formula.end();) {
            bool clauseContainsPureLiteral = false;
            for (int literal: *clauseIt) {
                int key = abs(literal);
                if (literalCount[key] != 0 &&
                    ((literal > 0 && literalCount[key] > 0) || (literal < 0 && literalCount[key] < 0))) {
                    clauseContainsPureLiteral = true;
                    break;
                }
            }
            if (clauseContainsPureLiteral) {
                clauseIt = formula.erase(clauseIt);
            } else {
                ++clauseIt;
            }
        }
    }


    int select_a_literal(const std::vector<int> &c, const std::vector<int> &cb) {
        for (int lit: c) {
            if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
                return lit;
            }
        }
        return 0;
    }


    void Vivify(CDNF_formula &cnf) {
        bool change = true;

        while (change) {
            change = false;
            UnitPropagation(cnf);
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
                    UP(cnf_backup, found_unit_clauses);
                    if (cnf_backup.empty()) {
                        cnf = CDNF_formula();
                        return; // found a sat solution
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
                                for (int lc: c) {
                                    if (lc != -found_unit_clauses[k].second) {
                                        new_clause.push_back(lc);
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
    }


    void VivifyWithPureLit(CDNF_formula &cnf) {
        bool change = true;

        while (change) {
            change = false;
            UnitPropagation(cnf);
            PureLiteralElimination(cnf);
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
                    UP(cnf_backup, found_unit_clauses);
                    if (cnf_backup.empty()) {
                        cnf = CDNF_formula();
                        return;
                    }
                    if (!cnf_backup.empty() && cnf_backup.front().empty()) {
                        cnf.insert(cnf.begin() + i, cb);
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else if (found_unit_clauses.size() > 1) {
                        if (found_unit_clauses[1].second > 0) {
                            if (cb.size() + 1 < c.size()) {// ToDO: check if it has to be contained in c
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
                                for (int lc: c) {
                                    if (lc != -found_unit_clauses[k].second) {
                                        new_clause.push_back(lc);
                                    }
                                }
                                cnf.insert(cnf.begin() + i, new_clause);
                                shortened = true;
                            }
                        }
                    }
                    PureLiteralElimination(cnf_backup);
                    if (cnf_backup.empty()) {
                        cnf = CDNF_formula();
                        return;
                    }
                }
                if (!shortened) {
                    cnf.insert(cnf.begin() + i, c);
                } else {
                    change = true;
                }
            }
        }
    }

}
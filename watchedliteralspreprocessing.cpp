//
// Created by Christofer Held on 25.04.24.
//

#include "watchedliteralspreprocessing.h"
#include <algorithm>

namespace watched_literals {

    void watched_literals_unit_propagation(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> unit_clauses;

        for (int i = 0; i < cnf.size(); i++) {
            if (cnf[i].size() > 1) {
                for (int j = 0; j < 2; j++) {
                    watchers[cnf[i][j]].push_back({i, j});
                }
            }
            if (cnf[i].size() == 1) {
                unit_clauses.push_back(cnf[i][0]);
            }
            if (cnf[i].size() <= 0) {
                cnf = {{}};
                return;
            }
        }
        if(unit_clauses.empty()) return;
        // we collect the info of which literals are propagated and apply later in O(n^2) time the knowledge
        std::set<int> propagated_literals;
        std::vector<bool> sat_clauses = std::vector<bool>(cnf.size());

        int count = 0;
        // now we check if we found unit clauses
        while (count < unit_clauses.size()) {
            propagated_literals.insert(unit_clauses[count]);
            // we set all clauses to true that are watched by unit
            if (watchers.contains(unit_clauses[count])) {
                for (const auto watcher: watchers[unit_clauses[count]]) {
                    sat_clauses[watcher.first] = true;
                }
                // this cannot be a watcher anymore
                // we don't replace the watchers due to the simple fact that the clause is already sat
                watchers.erase(unit_clauses[count]);
            }

            if (watchers.contains(-unit_clauses[count])) {
                // we have to replace the watchers for that literal
                for (const auto watcher: watchers[-unit_clauses[count]]) {
                    int clause = watcher.first;
                    // if the clause os already sat we don't care
                    if (sat_clauses[watcher.first]) continue;

                    int lit_pos = watcher.second + 1;
                    bool found_new_watcher = false;
                    int other_watcher_pos = watcher.second - 1;

                    while (lit_pos < cnf[clause].size()) {
                        // if next position is already a watcher
                        if (watchers.contains(cnf[clause][lit_pos]) &&
                            std::find(watchers[cnf[clause][lit_pos]].begin(),
                                      watchers[cnf[clause][lit_pos]].end(),
                                      std::pair<int, int>{clause, lit_pos}) !=
                            watchers[cnf[clause][lit_pos]].end()) {
                            other_watcher_pos = lit_pos; // we store that to speed up search
                            lit_pos++;
                            continue;
                        }
                        // if already the positive is propagated we can set the clause true and exit



                        // if already the negative is propagated continue
                        if (propagated_literals.contains(-cnf[clause][lit_pos])) {
                            lit_pos++;
                            continue;
                        }

                        // the literal is not watched, the literal is not propagated so we can use it for the
                        // new watched literal
                        found_new_watcher = true;

                        if (watchers.contains(cnf[clause][lit_pos])) {
                            watchers[cnf[clause][lit_pos]].push_back({clause, lit_pos});
                            break;
                        }

                        watchers[cnf[clause][lit_pos]] = {{clause, lit_pos}};
                        break;
                    }
                    if (!found_new_watcher) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other >= 0; other++)
                            if (watchers.contains(cnf[clause][other]) &&
                                std::find(watchers[cnf[clause][other]].begin(),
                                          watchers[cnf[clause][other]].end(),
                                          std::pair<int, int>{clause, lit_pos}) !=
                                watchers[cnf[clause][lit_pos]].end()) {

                                if (propagated_literals.contains(-cnf[clause][lit_pos])) {
                                    cnf = {{}}; // if conflict found
                                    return;
                                }

                                unit_clauses.push_back(cnf[clause][other]);
                                break;
                            }
                    }

                }
                watchers.erase(-unit_clauses[count]);
            }
            count++;
        }
        CDNF_formula new_formula;
        // now we need to apply the knowledge
        for(int i = 0; i < cnf.size(); i++) {
            if (sat_clauses[i]) continue;
            std::vector<int> cl;
            bool add = true;
            for (int j = 0; j < cnf[i].size(); j++){
                if (propagated_literals.contains(cnf[i][j])){
                    add = false;
                    break;
                }
                if (propagated_literals.contains(-cnf[i][j])){
                    continue;
                }
                cl.push_back(cnf[i][j]);
            }
            if (add){
                new_formula.push_back(std::move(cl));
            }
        }
        cnf = std::move(new_formula);
    }

    std::unordered_map<int, std::vector<std::pair<int, int>>> create_watched_literal_mapping(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        for (int i = 0; i < cnf.size(); i++) {
            for (int j = 0; j < 2; j++) {
                watchers[cnf[i][j]].push_back({i, j});
            }
        }
        return std::move(watchers);
    }

    int select_a_literal(const std::vector<int> &c, const std::vector<int> &cb) {
        for (int lit: c) {
            if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
                return lit;
            }
        }
        return 0;
    }

    struct runtime_info {
        std::set<int> propagated_literals;
        std::vector<bool> clauseIsSat;
    };

    void UP(std::unordered_map<int, std::vector<std::pair<int, int>>>& watchers,
            runtime_info &runtime_info,
            CDNF_formula &cnf,
            int new_unit,
            bool &is_unsat
    ) {
        std::vector<int> found_units;
        found_units.push_back(new_unit);
        while (!found_units.empty()) {
            // remove the currently found units from the to do list
            int current_unit = found_units[0];
            found_units.erase(found_units.begin());
            watched_literals_info.propagated.insert(current_unit);

            for (auto &clause: watched_literals_info.watchers[current_unit]) {
                std::
                unit_tracking.clauseIsSat[clause] = true;
            }

            for (int clause: cnf_mapping[-current_unit]) {
                if (unit_tracking.clauseIsSat[clause]) continue;
                if (!unit_tracking.clauseIsSat[clause]) {
                    unit_tracking.instances[clause] -= 1;
                }
                if (unit_tracking.instances[clause] == 1) {
                    int unit = 0;
                    for (int j: cnf[clause]) {
                        if (std::find(unit_tracking.propagated_literals.begin(),
                                      unit_tracking.propagated_literals.end(), -j)
                            == unit_tracking.propagated_literals.end()) {
                            unit = j;
                        }
                    }
                    // in that case we already propagated the counter example
                    if (std::find(found_units.begin(), found_units.end(), -unit) != found_units.end()) {
                        is_unsat = true;
                        return;
                    }
                    found_units.push_back(unit);
                }
            }
        }
    }


    void update_mapping(std::unordered_map<int, std::vector<int>> &mapping, std::vector<int> &c, std::vector<int> &cb,
                        int pos) {
        for (int lit: c) {
            bool find_pos = std::find(cb.begin(), cb.end(), lit) != cb.end();
            bool find_neg = std::find(cb.begin(), cb.end(), -lit) != cb.end();
            if (!find_neg && !find_pos) {
                auto newEnd = std::remove(mapping[lit].begin(), mapping[lit].end(), pos);
                mapping[lit].erase(newEnd, mapping[lit].end());
            }
        }
    }

    void vivify(CDNF_formula &cnf) {

        unit_propagation(cnf); // for the start we want to preprocess the clause to remove all unit clauses.
        bool change = true;

        // we store for faster Unit propagation
        std::unordered_map<int, std::vector<int>> cnf_mapping = create_literal_to_clause_mapping(cnf);

        while (change) {
            change = false;

            for (int i = 0; i < cnf.size(); i++) {
                // we now work with the tracking info no need to create
                runtime_info cnf_tracking = create_runtime_info(cnf);

                // we want to ignore the tracking info for now.
                cnf_tracking.clauseIsSat[i] = true;

                // we take a clause
                std::vector<int> c = cnf[i];

                // we take a finished clause
                std::vector<int> cb;

                bool shortened = false;

                while (!shortened && c != cb) {

                    int l = select_a_literal(c, cb);

                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat;
                    size_t num_propagations = cnf_tracking.propagated_literals.size();

                    UP(cnf_mapping, cnf_tracking, cnf, -l, is_unsat);

                    if (std::find(cnf_tracking.clauseIsSat.begin(), cnf_tracking.clauseIsSat.end(), false) ==
                        cnf_tracking.clauseIsSat.end()) {
                        cnf = CDNF_formula();
                        return;
                    }

                    if (is_unsat) {
                        update_mapping(cnf_mapping, c, cb, i);
                        cnf[i] = cb;
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for (size_t lit = num_propagations + 1; lit < cnf_tracking.propagated_literals.size(); lit++) {
                            int unit = cnf_tracking.propagated_literals[lit];

                            if (std::find(c.begin(), c.end(), unit) != c.end()) {
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    update_mapping(cnf_mapping, c, cb, i);
                                    cnf[i] = cb;
                                    shortened = true;
                                }
                                break;
                            }

                            if (std::find(c.begin(), c.end(), -unit) != c.end()) {
                                std::vector<int> new_clause;
                                for (int lc: c) {
                                    if (lc != -unit) {
                                        new_clause.push_back(lc);
                                    }
                                }
                                update_mapping(cnf_mapping, c, cb, i);
                                cnf[i] = cb;
                                shortened = true;
                                break;
                            }
                        }
                    }
                }
                if (!shortened) {
                    cnf_tracking.clauseIsSat[i] = false;
                } else {
                    change = true;
                    cnf_tracking.clauseIsSat[i] = false;
                }
            }
        }
    }

    void find_pures(std::unordered_map<int, std::vector<int>> &cnf_mapping, std::vector<bool> &clauses) {
        // Iterate over the map
        for (const auto &pair: cnf_mapping) {
            int key = pair.first;
            int neg_key = -key;

            // Check if the negated key does not exist in the map
            if (cnf_mapping.find(neg_key) == cnf_mapping.end()) {
                for (const auto &clause: pair.second) {
                    clauses[clause] = true;
                }
            }
        }
    }

    void vivify_with_pure_lit(CDNF_formula &cnf) {

        unit_propagation(cnf); // for the start we want to preprocess the clause to remove all unit clauses.
        bool change = true;

        // we store for faster Unit propagation
        std::unordered_map<int, std::vector<int>> cnf_mapping = create_literal_to_clause_mapping(cnf);
        std::vector<bool> contains_pures = std::vector<bool>(cnf.size(), false);
        find_pures(cnf_mapping, contains_pures);
        while (change) {
            change = false;

            if (std::find(contains_pures.begin(), contains_pures.end(), false) == contains_pures.end()) {
                cnf = CDNF_formula();
                return;
            }

            for (int i = 0; i < cnf.size(); i++) {
                if (contains_pures[i]) {
                    continue;
                }
                find_pures(cnf_mapping, contains_pures);
                // we now work with the tracking info no need to create
                runtime_info cnf_tracking = create_runtime_info(cnf);
                // we want to ignor clauses that are pure
                cnf_tracking.clauseIsSat = contains_pures;
                // we want to ignore the tracking info for now.
                cnf_tracking.clauseIsSat[i] = true;

                // we take a clause
                std::vector<int> c = cnf[i];

                // we take a finished clause
                std::vector<int> cb;

                bool shortened = false;

                while (!shortened && c != cb) {

                    int l = select_a_literal(c, cb);

                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat;
                    size_t num_propagations = cnf_tracking.propagated_literals.size();

                    UP(cnf_mapping, cnf_tracking, cnf, -l, is_unsat);

                    if (std::find(cnf_tracking.clauseIsSat.begin(), cnf_tracking.clauseIsSat.end(), false) ==
                        cnf_tracking.clauseIsSat.end()) {
                        cnf = CDNF_formula();
                        return;
                    }

                    if (is_unsat) {
                        update_mapping(cnf_mapping, c, cb, i);
                        cnf[i] = cb;
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for (size_t lit = num_propagations + 1; lit < cnf_tracking.propagated_literals.size(); lit++) {
                            int unit = cnf_tracking.propagated_literals[lit];

                            if (std::find(c.begin(), c.end(), unit) != c.end()) {
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    update_mapping(cnf_mapping, c, cb, i);
                                    cnf[i] = cb;
                                    shortened = true;
                                }
                                break;
                            }

                            if (std::find(c.begin(), c.end(), -unit) != c.end()) {
                                std::vector<int> new_clause;
                                for (int lc: c) {
                                    if (lc != -unit) {
                                        new_clause.push_back(lc);
                                    }
                                }
                                update_mapping(cnf_mapping, c, cb, i);
                                cnf[i] = cb;
                                shortened = true;
                                break;
                            }
                        }
                    }
                }
                if (!shortened) {
                    cnf_tracking.clauseIsSat[i] = false;
                } else {
                    change = true;
                    cnf_tracking.clauseIsSat[i] = false;
                }
            }
        }

        for (size_t i = contains_pures.size() - 1; i >= 0; i--) {
            if (contains_pures[i]) {
                cnf.erase(cnf.begin() + i);
            }
        }
    }
*/
} // watched_literals
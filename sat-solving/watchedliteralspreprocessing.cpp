//
// Created by Christofer Held on 25.04.24.
//

#include "watchedliteralspreprocessing.h"

namespace watched_literals {

    void watched_literals_unit_propagation(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> unit_clauses;

        for (int i = 0; i < cnf.size(); i++) {
            if (cnf[i].size() > 1) {
                for (int j = 0; j < 2; j++) {
                    watchers[cnf[i][j]].emplace_back(i, j);
                }
            }
            if (cnf[i].size() == 1) {
                unit_clauses.push_back(cnf[i][0]);
            }
            if (cnf[i].empty()) {
                cnf = {{}};
                return;
            }
        }
        if (unit_clauses.empty()) return;
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
                        if (propagated_literals.contains(cnf[clause][lit_pos])) {
                            sat_clauses[clause] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (propagated_literals.contains(-cnf[clause][lit_pos])) {
                            lit_pos++;
                            continue;
                        }

                        // the literal is not watched, the literal is not propagated so we can use it for the
                        // new watched literal
                        found_new_watcher = true;

                        if (watchers.contains(cnf[clause][lit_pos])) {
                            watchers[cnf[clause][lit_pos]].emplace_back(clause, lit_pos);
                            break;
                        }

                        watchers[cnf[clause][lit_pos]] = {{clause, lit_pos}};
                        break;
                    }
                    if (!found_new_watcher) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other >= 0; other++) {
                            if (propagated_literals.contains(cnf[clause][other])) {
                                sat_clauses[clause] = true;
                                break;
                            }

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

                }
                watchers.erase(-unit_clauses[count]);
            }
            count++;
        }
        CDNF_formula new_formula;
        // now we need to apply the knowledge
        for (int i = 0; i < cnf.size(); i++) {
            if (sat_clauses[i]) continue;
            std::vector<int> cl;
            bool add = true;
            for (int j: cnf[i]) {
                if (propagated_literals.contains(j)) {
                    add = false;
                    break;
                }
                if (propagated_literals.contains(-j)) {
                    continue;
                }
                cl.push_back(j);
            }
            if (add) {
                new_formula.push_back(std::move(cl));
            }
        }
        cnf = std::move(new_formula);
    }

    std::unordered_map<int, std::vector<std::pair<int, int>>> create_watched_literal_mapping(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        for (int i = 0; i < cnf.size(); i++) {
            for (int j = 0; j < 2; j++) {
                watchers[cnf[i][j]].emplace_back(i, j);
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
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> propagated_literals;
        std::vector<bool> clauseIsSat;
    };

    runtime_info create_runtime_info(const CDNF_formula &cnf,
                                     const std::unordered_map<int, std::vector<std::pair<int, int>>> &watchers) {
        runtime_info info;
        info.clauseIsSat = std::vector<bool>(cnf.size());
        info.watchers = watchers;
        return std::move(info);
    }


    void UP(runtime_info &runtime_info,
            const CDNF_formula &cnf,
            int new_unit,
            bool &is_unsat
    ) {
        std::vector<int> unit_clauses = {new_unit};

        int count = 0;
        while (count < unit_clauses.size()) {
            runtime_info.propagated_literals.push_back(unit_clauses[count]);

            if (runtime_info.watchers.contains(unit_clauses[count])) {
                for (const auto watcher: runtime_info.watchers[unit_clauses[count]]) {
                    runtime_info.clauseIsSat[watcher.first] = true;
                }
                runtime_info.watchers.erase(unit_clauses[count]);
            }

            if (runtime_info.watchers.contains(-unit_clauses[count])) {
                // we have to replace the watchers for that literal
                for (const auto watcher: runtime_info.watchers[-unit_clauses[count]]) {
                    int clause = watcher.first;
                    // if the clause os already sat we don't care
                    if (runtime_info.clauseIsSat[watcher.first]) continue;

                    int lit_pos = watcher.second + 1;
                    bool found_new_watcher = false;
                    int other_watcher_pos = watcher.second - 1;

                    while (lit_pos < cnf[clause].size()) {
                        // if next position is already a watcher
                        if (runtime_info.watchers.contains(cnf[clause][lit_pos]) &&
                            std::find(runtime_info.watchers[cnf[clause][lit_pos]].begin(),
                                      runtime_info.watchers[cnf[clause][lit_pos]].end(),
                                      std::pair<int, int>{clause, lit_pos}) !=
                            runtime_info.watchers[cnf[clause][lit_pos]].end()) {
                            other_watcher_pos = lit_pos; // we store that to speed up search
                            lit_pos++;
                            continue;
                        }
                        // if already the positive is propagated we can set the clause true and exit
                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      cnf[clause][lit_pos]) != runtime_info.propagated_literals.end()) {
                            runtime_info.clauseIsSat[clause] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      -cnf[clause][lit_pos]) != runtime_info.propagated_literals.end()) {
                            lit_pos++;
                            continue;
                        }

                        // the literal is not watched, the literal is not propagated so we can use it for the
                        // new watched literal
                        found_new_watcher = true;

                        if (runtime_info.watchers.contains(cnf[clause][lit_pos])) {
                            runtime_info.watchers[cnf[clause][lit_pos]].emplace_back(clause, lit_pos);
                            break;
                        }

                        runtime_info.watchers[cnf[clause][lit_pos]] = {{clause, lit_pos}};
                        break;
                    }
                    if (!found_new_watcher) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other >= 0; other++) {

                            if (std::find(runtime_info.propagated_literals.begin(),
                                          runtime_info.propagated_literals.end(),
                                          cnf[clause][other]) != runtime_info.propagated_literals.end()) {
                                runtime_info.clauseIsSat[clause] = true;
                                break;
                            }

                            if (runtime_info.watchers.contains(cnf[clause][other]) &&
                                std::find(runtime_info.watchers[cnf[clause][other]].begin(),
                                          runtime_info.watchers[cnf[clause][other]].end(),
                                          std::pair<int, int>{clause, lit_pos}) !=
                                runtime_info.watchers[cnf[clause][lit_pos]].end()) {

                                if (std::find(runtime_info.propagated_literals.begin(),
                                              runtime_info.propagated_literals.end(),
                                              -cnf[clause][lit_pos]) != runtime_info.propagated_literals.end()) {
                                    is_unsat = true;
                                    return;
                                }

                                unit_clauses.push_back(cnf[clause][other]);
                                break;
                            }
                        }
                    }

                }
                runtime_info.watchers.erase(-unit_clauses[count]);
            }
            count++;
        }
    }

    void update_watchers(std::unordered_map<int, std::vector<std::pair<int, int>>> &mapping, std::vector<int> &c,
                         std::vector<int> &cb,
                         int pos) {
        for (int i = 0; i < c.size(); i++) {
            if (mapping.contains(c[i]) &&
                std::find(mapping[c[i]].begin(), mapping[c[i]].end(), std::pair<int, int>{pos, i}) !=
                mapping[c[i]].end()) {
                auto newEnd = std::remove(mapping[c[i]].begin(), mapping[c[i]].end(), std::pair<int, int>{pos, i});
                mapping[c[i]].erase(newEnd, mapping[c[i]].end());
            }
        }
        if (mapping.contains(cb[cb.size() - 1])) {
            mapping[cb[cb.size() - 1]].emplace_back(pos, cb.size() - 1);
        } else {
            mapping[cb[cb.size() - 1]] = {{pos, cb.size() - 1}};
        }

        if (mapping.contains(cb[cb.size() - 2])) {
            mapping[cb[cb.size() - 2]].emplace_back(pos, cb.size() - 2);
        } else {
            mapping[cb[cb.size() - 2]] = {{pos, cb.size() - 2}};
        }

    }

    int numLiterals(const CDNF_formula &cnf) {
        std::set<int> literals;
        for (const auto &clause: cnf) {
            for (const auto &lit: clause) {
                literals.insert(std::abs(lit));
            }
        }
        return literals.size();
    }

    int numVars(const CDNF_formula &cnf) {
        int literals = 0;
        for (const auto &clause: cnf) {
            literals += clause.size();
        }
        return literals;
    }


    void vivify(CDNF_formula &cnf, int global_count, CSVChangeWriter &writer) {
        // for the start we want to preprocess the clause to remove all unit clauses.
        watched_literals_unit_propagation(cnf);
        int change = 0;
        int new_change = cnf.size();

        // we store for faster Unit propagation
        auto watchers = create_watched_literal_mapping(cnf);
        int step = 0;
        while (new_change > 0) {
            writer.writeData(global_count, step, cnf.size(), (int) (change / 2), numVars(cnf), numLiterals(cnf));
            new_change = 0;
            change = 0;
            step++;
            for (int i = 0; i < cnf.size(); i++) {
                if(i % 1000 == 0) {
                    std::cout << "interation: " << i << " \tclause: " << i << "\t changed: " << change / 2 << std::endl;
                }
                // we now work with the tracking info no need to create
                runtime_info cnf_tracking = create_runtime_info(cnf, watchers);

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

                    UP(cnf_tracking, cnf, -l, is_unsat);

                    if (std::find(cnf_tracking.clauseIsSat.begin(), cnf_tracking.clauseIsSat.end(), false) ==
                        cnf_tracking.clauseIsSat.end()) {
                        cnf = CDNF_formula();
                        return;
                    }

                    if (is_unsat) {
                        update_watchers(watchers, c, cb, i);
                        cnf[i] = cb;
                        if (c != cb) {
                            shortened = true;
                            change++;
                        }
                    } else {
                        for (size_t lit = num_propagations + 1; lit < cnf_tracking.propagated_literals.size(); lit++) {
                            int unit = cnf_tracking.propagated_literals[lit];

                            if (std::find(c.begin(), c.end(), unit) != c.end()) {
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    update_watchers(watchers, c, cb, i);
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
                                update_watchers(watchers, c, cb, i);
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
                    change++;
                    cnf_tracking.clauseIsSat[i] = false;
                }
            }
            new_change = change;
        }
    }

} // watched_literals
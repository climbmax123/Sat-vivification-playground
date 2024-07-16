//
// Created by christofer on 26.04.24.
//

#include "vivify.h"
#include <iostream>

namespace vivify {

    struct runtime_info {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> propagated_literals;
        std::vector<bool> clauseIsSat;
    };

    runtime_info create_runtime_info(QBF &qbf) {
        runtime_info info;
        info.clauseIsSat = std::vector<bool>(qbf.formula.size());

        for (int i = 0; i < qbf.formula.size(); i++) {
            // We know here that we don't contain units and emtpy clauses
            auto t = qbf.formula[i].size() - 1;
            while (t > 0) {
                if (qbf.quantifierTypeIsExists[abs(qbf.formula[i][t])]) {
                    info.watchers[qbf.formula[i][t]].emplace_back(i, t);
                    info.watchers[qbf.formula[i][t - 1]].emplace_back(i, t - 1);
                    break;
                }
                t--;
            }
        }
        return std::move(info);
    }

    int select_a_literal(QBF &qbf, const std::vector<int> &c, const std::vector<int> &cb) {
        for (int lit: c) {
            if (qbf.quantifierTypeIsExists[abs(lit)]) {
                if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
                    return lit;
                }
            }
        }
        return 0;
    }


    void UP(runtime_info &runtime_info,
            QBF &qbf,
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

                for (const auto watcher: runtime_info.watchers[-unit_clauses[count]]) {
                    if (runtime_info.clauseIsSat[watcher.first]) continue;

                    int clause = watcher.first;
                    int lit_pos = watcher.second - 1;
                    bool found_new_watcher = false;
                    int other_watcher_pos = watcher.second + 1;

                    while (lit_pos >= 0) {

                        if (runtime_info.watchers.contains(qbf.formula[clause][lit_pos]) &&
                            std::find(runtime_info.watchers[qbf.formula[clause][lit_pos]].begin(),
                                      runtime_info.watchers[qbf.formula[clause][lit_pos]].end(),
                                      std::pair<int, int>{clause, lit_pos}) !=
                            runtime_info.watchers[qbf.formula[clause][lit_pos]].end()) {
                            other_watcher_pos = lit_pos;
                            lit_pos--;
                            continue;
                        }

                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      qbf.formula[clause][lit_pos]) != runtime_info.propagated_literals.end()) {
                            runtime_info.clauseIsSat[clause] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      -qbf.formula[clause][lit_pos]) != runtime_info.propagated_literals.end()) {
                            lit_pos--;
                            continue;
                        }

                        // the literal is not watched, the literal is not propagated so we can use it for the
                        // new watched literal
                        found_new_watcher = true;

                        if (runtime_info.watchers.contains(qbf.formula[clause][lit_pos])) {
                            runtime_info.watchers[qbf.formula[clause][lit_pos]].emplace_back(clause, lit_pos);
                            break;
                        }

                        runtime_info.watchers[qbf.formula[clause][lit_pos]] = {{clause, lit_pos}};
                        break;
                    }
                    if (!found_new_watcher && !runtime_info.clauseIsSat[clause]) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other < qbf.formula[clause].size(); other++) {
                            if (std::find(runtime_info.propagated_literals.begin(),
                                          runtime_info.propagated_literals.end(), qbf.formula[clause][other]) !=
                                runtime_info.propagated_literals.end()) {
                                runtime_info.clauseIsSat[clause] = true;
                                break;
                            }

                            if (runtime_info.watchers.contains(qbf.formula[clause][other]) &&
                                std::find(runtime_info.watchers[qbf.formula[clause][other]].begin(),
                                          runtime_info.watchers[qbf.formula[clause][other]].end(),
                                          std::pair<int, int>{clause, other}) !=
                                runtime_info.watchers[qbf.formula[clause][other]].end()) {

                                if (std::find(unit_clauses.begin(), unit_clauses.end(), -qbf.formula[clause][other]) !=
                                    unit_clauses.end()) {
                                    is_unsat = true; // if conflict found
                                    return;
                                }

                                if (!qbf.quantifierTypeIsExists[std::abs(qbf.formula[clause][other])]) {
                                    is_unsat = true;
                                    return;
                                }

                                unit_clauses.push_back(qbf.formula[clause][other]);
                                break;
                            }
                        }
                    }

                }
                runtime_info.watchers.erase(-unit_clauses[count]);
            }
            count++;

            int falseCount = std::count(runtime_info.clauseIsSat.begin(), runtime_info.clauseIsSat.end(), false);
            if (falseCount == 1) {
                for (int i = 0; i < runtime_info.clauseIsSat.size(); i++) {
                    if (!runtime_info.clauseIsSat[i]) {
                        for (int j = 0; j < qbf.formula[i].size(); j++) {
                            if (qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][j])] &&
                                (std::find(runtime_info.propagated_literals.begin(),
                                           runtime_info.propagated_literals.end(), -qbf.formula[i][j]) ==
                                 runtime_info.propagated_literals.end())) {
                                runtime_info.clauseIsSat[i] = true;
                            }
                        }
                    }
                }
            }
        }
    }

    // TODO: Check
    std::vector<int>
    update_watchers(runtime_info &runtimeInfo, QBF &qbf, std::vector<int> &c, std::vector<int> &cb, int i,
                    bool &is_unsat) {
        std::vector<int> new_c;
        // Add the literals
        for (int j: c) {
            if (!qbf.quantifierTypeIsExists[abs(j)]) {
                new_c.push_back(j);
                continue;
            }

            if (std::find(cb.begin(), cb.end(), j) != cb.end()) {
                new_c.push_back(j);
            }
        }
        // Apply universal reduction
        for (int j = new_c.size() - 1; j >= 0; j--) {
            if (!qbf.quantifierTypeIsExists[abs(new_c[j])]) {
                new_c.pop_back();
            } else {
                break;
            }
        }

        if (new_c.empty()) {
            is_unsat = true;
            return {};
        }

        // Delete watchers
        for (int j = 0; j < c.size(); j++) {
            if (runtimeInfo.watchers.contains(c[j])) {
                auto newEnd = std::remove(runtimeInfo.watchers[c[j]].begin(), runtimeInfo.watchers[c[j]].end(),
                                          std::pair<int, int>{i, j});
                runtimeInfo.watchers[c[j]].erase(newEnd, runtimeInfo.watchers[c[j]].end());
            }
        }
        // Create new Watchers for the clause
        if (new_c.size() >= 2) {
            runtimeInfo.watchers[new_c.size() - 1].emplace_back(i, new_c.size() - 1);
            runtimeInfo.watchers[new_c.size() - 2].emplace_back(i, new_c.size() - 1 - 1);
        }

        return std::move(new_c);
    }


    bool is_equal_on_exists(QBF &qbf, std::vector<int> c, std::vector<int> cb) {
        for (int lit: c) {
            if (qbf.quantifierTypeIsExists[abs(lit)]) {
                if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
                    return false;
                }
            }
        }
        return true;
    }


    void printQBF(QBF &qbf) {
        for (const auto &qt: qbf.quantifierOrder) {
            if (qbf.quantifierTypeIsExists[qt]) {
                std::cout << "E ";
            } else if (!qbf.quantifierTypeIsExists[qt]) {
                std::cout << "A ";
            }
            std::cout << qt << " ";
        }
        std::cout << std::endl;

        for (const auto &clause: qbf.formula) {
            std::cout << "(";
            for (const auto &literal: clause) {
                std::cout << literal << " ";
            }
            std::cout << ") ";
        }
        std::cout << std::endl;
    }


    void watched_literals_vivify(QBF &qbf, int timeLimitInSeconds) {
        auto startTime = std::chrono::steady_clock::now();
        unit::watched_literals_unit_propagation_with_ur(qbf, timeLimitInSeconds);
        // create check for change
        bool change = true;

        // If qbf is sat or unsat after unit propagation
        if (qbf.formula.empty() || qbf.formula[0].empty()) {
            return;
        }

        while (change) {
            change = false;
            auto runtimeInfo = create_runtime_info(qbf);

            for (int i = 0; i < qbf.formula.size(); i++) {
                if (timeLimitInSeconds != -1) {
                    auto currentTime = std::chrono::steady_clock::now();
                    auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                            currentTime - startTime).count();
                    if (elapsedSeconds >= timeLimitInSeconds) {
                        std::cout << "Timeout reached ending vivify" << std::endl;
                        return;
                    }
                }


                // we work with a copy of the runtimeInfo. And adapt this after each step
                runtime_info qbf_tracking = runtimeInfo;

                std::cout << i << std::endl;
                printQBF(qbf);

                // we want to ignore the tracking info for now.
                qbf_tracking.clauseIsSat[i] = true;

                // we take a clause
                std::vector<int> c = qbf.formula[i];

                // we take a finished clause
                std::vector<int> cb;

                bool shortened = false;
                bool found_unit = false;
                while (!shortened && !is_equal_on_exists(qbf, c, cb)) {

                    int l = select_a_literal(qbf, c, cb);

                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat = false;
                    size_t num_propagations = qbf_tracking.propagated_literals.size();
                    UP(qbf_tracking, qbf, -l, is_unsat);

                    if (std::find(qbf_tracking.clauseIsSat.begin(), qbf_tracking.clauseIsSat.end(), false) ==
                        qbf_tracking.clauseIsSat.end()) {
                        if (!is_equal_on_exists(qbf, c, cb)) {
                            std::cout << "SAT" << std::endl;
                            qbf.formula = {};
                            return;
                        }
                    }

                    if (is_unsat) {
                        bool new_unsat = false;
                        qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i, new_unsat);
                        if (new_unsat) {
                            qbf.formula = {{}};
                            return;
                        }
                        // if(qbf.formula[i].size() == 1){
                        // found new unit
                        //     std::cout << qbf.formula.size() << std::endl;
                        //     unit::watched_literals_unit_propagation_without_ur(qbf);
                        //    std::cout << qbf.formula.size() << std::endl;
                        //   runtimeInfo = create_runtime_info(qbf);
                        //  found_unit = true;
                        //     shortened = true;
                        //     change = true;
                        //    break;
                        //}

                        if (!is_equal_on_exists(qbf, c, cb)) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for (size_t lit = num_propagations + 1; lit < qbf_tracking.propagated_literals.size(); lit++) {
                            int unit = qbf_tracking.propagated_literals[lit];

                            if (std::find(c.begin(), c.end(), unit) != c.end()) {
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    bool is_unsat = false;
                                    qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i, is_unsat);
                                    if (is_unsat) {
                                        qbf.formula = {{}};
                                        return;
                                    }
                                    //if(qbf.formula[i].size() == 1){
                                    //    // found new unit
                                    //    unit::watched_literals_unit_propagation_with_ur(qbf);
                                    //    runtimeInfo = create_runtime_info(qbf);
                                    //    found_unit = true;
                                    //}
                                    if (!is_equal_on_exists(qbf, c, cb)) {
                                        shortened = true;
                                    }
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
                                bool is_unsat = false;
                                qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i, is_unsat);
                                if (is_unsat) {
                                    qbf.formula = {{}};
                                    return;
                                }
                                if (qbf.formula[i].size() == 1) {
                                    // found new unit
                                    // unit::watched_literals_unit_propagation_with_ur(qbf);
                                    //runtimeInfo = create_runtime_info(qbf);
                                    //found_unit = true;
                                }
                                if (!is_equal_on_exists(qbf, c, cb)) {
                                    shortened = true;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (!shortened) {
                    qbf_tracking.clauseIsSat[i] = false;
                } else {
                    change = true;
                    qbf_tracking.clauseIsSat[i] = false;
                }
                //if(found_unit){
                //    change = true;
                //    break;
                //}
            }
        }
    }

}
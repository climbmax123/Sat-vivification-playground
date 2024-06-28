//
// Created by christofer on 26.04.24.
//

#include "vivify.h"
#include <iostream>

namespace vivify {

    void watched_literals_unit_propagation(QBF &qbf){

    }

    void watched_literals_unit_propagation_with_universal_reduction(QBF &qbf) {

        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers; // clause, lit

        std::vector<int> unit_clauses;

        // ----------------------------------------
        // Prepare Watched literals
        // ----------------------------------------

        for (int i = 0; i < qbf.formula.size(); i++) {
            // Find 2 watchers (the most right ones)
            if (qbf.formula[i].size() > 1) {
                auto t = qbf.formula[i].size() - 1;
                while (t > 0) {
                    if (qbf.quantifierTypeIsExists[abs(qbf.formula[i][t])]) {
                        watchers[qbf.formula[i][t]].emplace_back(i, t);
                        watchers[qbf.formula[i][t - 1]].emplace_back(i, t - 1);
                        break;
                    }
                    t--;
                }
                // With universal reduction this may be a unit clause
                if (t == 0) {
                    if (qbf.quantifierTypeIsExists[abs(qbf.formula[i][0])]) {
                        unit_clauses.push_back(qbf.formula[i][0]);
                    } else {
                        qbf.formula = {{}};
                        return;
                    }
                }
            }
            if (qbf.formula[i].size() == 1) {
                if (qbf.quantifierTypeIsExists[abs(qbf.formula[i][0])]) {
                    unit_clauses.push_back(qbf.formula[i][0]);
                } else {
                    qbf.formula = {{}}; // formula is false only one universal lit in over
                    return;
                }
            }
            if (qbf.formula[i].empty()) {
                qbf.formula = {{}};
                return;
            }
        }

        // if we don't find any unit clauses we are done
        if (unit_clauses.empty()) return;
        // ----------------------------------------
        // Apply Unit Propagation
        // ----------------------------------------

        // we collect the info of which literals are propagated and apply later in O(n^2) time the knowledge
        std::set<int> propagated_literals;
        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        int count = 0;

        while (count < unit_clauses.size()) {
            propagated_literals.insert(unit_clauses[count]);

            // we set all clauses to true that are watched by unit this can only be existential literals
            if (watchers.contains(unit_clauses[count])) {
                for (const auto watcher: watchers[unit_clauses[count]]) {
                    sat_clauses[watcher.first] = true;
                }
                // this cannot be a watcher anymore
                // we don't replace the watchers due to the simple fact that the clause is already sat
                watchers.erase(unit_clauses[count]);
            }

            // we have to update all watchers that watch -l and reset them
            if (watchers.contains(-unit_clauses[count])) {

                // we have to replace the watchers for that literal
                for (const auto watcher: watchers[-unit_clauses[count]]) {
                    // if the clause os already sat we don't care
                    if (sat_clauses[watcher.first]) continue;

                    // We start with the next literal
                    int lit_pos = watcher.second - 1;

                    bool found_new_watcher = false;

                    int other_watcher_pos = watcher.second + 1; // we want to find the other watcher quickly.

                    // We go the other way around from current pos to 0
                    while (lit_pos >= 0) {

                        // if next position is already the other Watcher
                        if (watchers.contains(qbf.formula[watcher.first][lit_pos]) &&
                            std::find(watchers[qbf.formula[watcher.first][lit_pos]].begin(),
                                      watchers[qbf.formula[watcher.first][lit_pos]].end(),
                                      std::pair<int, int>{watcher.first, lit_pos}) !=
                            watchers[qbf.formula[watcher.first][lit_pos]].end()) {

                            // The other Watcher must be existential if we overstep it. Otw. we have to rest it
                            if (!qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])]) {
                                // remove the watcher
                                watchers[qbf.formula[watcher.first][lit_pos]]
                                        .erase(std::find(watchers[qbf.formula[watcher.first][lit_pos]].begin(),
                                                         watchers[qbf.formula[watcher.first][lit_pos]].end(),
                                                         std::pair<int, int>{watcher.first, lit_pos}));

                                lit_pos--;
                                // find the next existential watcher
                                while (lit_pos >= 0) {
                                    // check if we have found a positive existential
                                    if (propagated_literals.contains(qbf.formula[watcher.first][lit_pos])) {
                                        sat_clauses[watcher.first] = true;
                                        break;
                                    }
                                    // check if literal is already assigned
                                    if (propagated_literals.contains(-qbf.formula[watcher.first][lit_pos])) {
                                        lit_pos--;
                                        continue;
                                    }
                                    // check if literal is existential
                                    if (!qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])] ) {
                                        lit_pos--;
                                        continue;
                                    }

                                    // now we have found the next existential unassigned literal
                                    other_watcher_pos = lit_pos;
                                    if (watchers.contains(qbf.formula[watcher.first][lit_pos])) {
                                        watchers[qbf.formula[watcher.first][lit_pos]].emplace_back(watcher.first,
                                                                                                   lit_pos);
                                        break;
                                    }
                                    watchers[qbf.formula[watcher.first][lit_pos]] = {{watcher.first, lit_pos}};

                                }
                                if (sat_clauses[watcher.first]) {
                                    break;
                                }
                                // If universal reduction would yield an empty clause
                                if (lit_pos < 0) {
                                    qbf.formula = {{}};
                                    return;
                                }
                            }

                            lit_pos--;
                            continue;
                        }

                        // if already the positive is propagated existential literal. In general this should always be existential
                        if (propagated_literals.contains(qbf.formula[watcher.first][lit_pos])
                            && qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])]) {
                            sat_clauses[watcher.first] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (propagated_literals.contains(-qbf.formula[watcher.first][lit_pos])) {
                            lit_pos--;
                            continue;
                        }

                        // We know that the first watcher is before this watcher and existential
                        // If this would be the one behind we would have overstepped the watcher and reset it
                        // If this would be the one before we know that the other is existential
                        // Therefore we can pick any literal here.
                        found_new_watcher = true;
                        if (watchers.contains(qbf.formula[watcher.first][lit_pos])) {
                            watchers[qbf.formula[watcher.first][lit_pos]].emplace_back(watcher.first, lit_pos);
                            break;
                        }

                        watchers[qbf.formula[watcher.first][lit_pos]] = {{watcher.first, lit_pos}};
                    }

                    // In this case we have found only one watcher and therefore a new unit clause
                    if (!found_new_watcher) {
                        for (int other = other_watcher_pos; other < qbf.formula[watcher.first].size(); other++) {
                            if (propagated_literals.contains(qbf.formula[watcher.first][other])) {
                                sat_clauses[watcher.first] = true;
                                break;
                            }

                            if (watchers.contains(qbf.formula[watcher.first][other]) &&
                                std::find(watchers[qbf.formula[watcher.first][other]].begin(),
                                          watchers[qbf.formula[watcher.first][other]].end(),
                                          std::pair<int, int>{watcher.first, lit_pos}) !=
                                watchers[qbf.formula[watcher.first][lit_pos]].end()) {

                                // If we have a conflict form unit propagation
                                if (propagated_literals.contains(-qbf.formula[watcher.first][lit_pos])) {
                                    qbf.formula = {{}}; // if conflict found
                                    return;
                                }

                                unit_clauses.push_back(qbf.formula[watcher.first][other]);
                                break;
                            }
                        }
                    }

                }
                watchers.erase(-unit_clauses[count]);
            }
            count++;
        }
        std::vector<std::vector<int>> new_formula;
        // now we need to apply the knowledge
        for (int i = 0; i < qbf.formula.size(); i++) {
            if (sat_clauses[i]) continue;
            std::vector<int> cl;
            bool add = true;
            bool reduce = true;
            for (int j = qbf.formula[i].size() - 1; j >= 0; j--) {
                if (reduce && !qbf.quantifierTypeIsExists[abs(qbf.formula[i][j])]) {
                    continue;
                }
                if (propagated_literals.contains(qbf.formula[i][j])) {
                    add = false;
                    break;
                }
                if (propagated_literals.contains(-qbf.formula[i][j])) {
                    continue;
                }
                reduce = false;
                cl.insert(cl.begin(), qbf.formula[i][j]);
            }
            if (add) {
                new_formula.push_back(std::move(cl));
            }
        }
        std::vector<int> newQuantifierOrder;
        std::unordered_map<int, bool> newQuantors;
        // Now we reduce the quantors
        for (int element: qbf.quantifierOrder) {
            bool found = false;
            for (const auto &sub_vector: new_formula) {
                if (std::find(sub_vector.begin(), sub_vector.end(), element) != sub_vector.end()) {
                    found = true;
                    break;
                }
            }
            if (found) {
                newQuantifierOrder.push_back(element);
                newQuantors[element] = qbf.quantifierTypeIsExists[abs(element)];
            }
        }

        qbf.formula = std::move(new_formula);
        qbf.quantifierTypeIsExists = std::move(newQuantors);
        qbf.quantifierOrder = std::move(newQuantifierOrder);
    }


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

            // we set all clauses to true that are watched by unit this can only be existential literals
            if (runtime_info.watchers.contains(unit_clauses[count])) {
                for (const auto watcher: runtime_info.watchers[unit_clauses[count]]) {
                    runtime_info.clauseIsSat[watcher.first] = true;
                }
                // this cannot be a watcher anymore
                // we don't replace the watchers due to the simple fact that the clause is already sat
                runtime_info.watchers.erase(unit_clauses[count]);
            }

            // we have to update all watchers that watch -l and reset them
            if (runtime_info.watchers.contains(-unit_clauses[count])) {

                // we have to replace the watchers for that literal
                for (const auto watcher: runtime_info.watchers[-unit_clauses[count]]) {
                    // if the clause os already sat we don't care
                    if (runtime_info.clauseIsSat[watcher.first]) continue;

                    // We start with the next literal
                    int lit_pos = watcher.second - 1;

                    bool found_new_watcher = false;

                    int other_watcher_pos = watcher.second + 1; // we want to find the other watcher quickly.

                    // We go the other way around from current pos to 0
                    while (lit_pos >= 0) {

                        // if next position is already the other Watcher
                        if (runtime_info.watchers.contains(qbf.formula[watcher.first][lit_pos]) &&
                            std::find(runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].begin(),
                                      runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].end(),
                                      std::pair<int, int>{watcher.first, lit_pos}) !=
                            runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].end()) {

                            // The other Watcher must be existential if we overstep it. Otw. we have to rest it
                            if (!qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])]) {
                                // remove the watcher
                                runtime_info.watchers[qbf.formula[watcher.first][lit_pos]]
                                        .erase(std::find(
                                                runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].begin(),
                                                runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].end(),
                                                std::pair<int, int>{watcher.first, lit_pos}));

                                lit_pos--;
                                // find the next existential watcher
                                while (lit_pos >= 0) {
                                    // check if we have found a positive existential
                                    if (std::find(runtime_info.propagated_literals.begin(),
                                                  runtime_info.propagated_literals.end(),
                                                  qbf.formula[watcher.first][lit_pos])
                                        != runtime_info.propagated_literals.end()
                                            ) {
                                        runtime_info.clauseIsSat[watcher.first] = true;
                                        break;
                                    }
                                    // check if literal is already assigned
                                    if (std::find(runtime_info.propagated_literals.begin(),
                                                  runtime_info.propagated_literals.end(),
                                                  -qbf.formula[watcher.first][lit_pos])
                                        != runtime_info.propagated_literals.end()) {
                                        lit_pos--;
                                        continue;
                                    }
                                    // check if literal is existential
                                    if (!qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])]) {
                                        lit_pos--;
                                        continue;
                                    }

                                    // now we have found the next existential unassigned literal
                                    other_watcher_pos = lit_pos;
                                    if (runtime_info.watchers.contains(qbf.formula[watcher.first][lit_pos])) {
                                        runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].emplace_back(
                                                watcher.first,
                                                lit_pos);
                                        break;
                                    }
                                    runtime_info.watchers[qbf.formula[watcher.first][lit_pos]] = {
                                            {watcher.first, lit_pos}};

                                }
                                if (runtime_info.clauseIsSat[watcher.first]) {
                                    break;
                                }
                                // If universal reduction would yield an empty clause
                                if (lit_pos < 0) {
                                    is_unsat = true;
                                    return;
                                }
                            }

                            lit_pos--;
                            continue;
                        }

                        // if already the positive is propagated existential literal. In general this should always be existential
                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      qbf.formula[watcher.first][lit_pos]) != runtime_info.propagated_literals.end()
                            && qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][lit_pos])]) {
                            runtime_info.clauseIsSat[watcher.first] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (std::find(runtime_info.propagated_literals.begin(), runtime_info.propagated_literals.end(),
                                      -qbf.formula[watcher.first][lit_pos]) != runtime_info.propagated_literals.end()) {
                            lit_pos--;
                            continue;
                        }

                        // We know that the first watcher is before this watcher and existential
                        // If this would be the one behind we would have overstepped the watcher and reset it
                        // If this would be the one before we know that the other is existential
                        // Therefore we can pick any literal here.
                        found_new_watcher = true;
                        if (runtime_info.watchers.contains(qbf.formula[watcher.first][lit_pos])) {
                            runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].emplace_back(watcher.first,
                                                                                                    lit_pos);
                            break;
                        }

                        runtime_info.watchers[qbf.formula[watcher.first][lit_pos]] = {{watcher.first, lit_pos}};
                    }

                    // In this case we have found only one watcher and therefore a new unit clause
                    if (!found_new_watcher) {
                        for (int other = other_watcher_pos; other < qbf.formula[watcher.first].size(); other++) {
                            if (std::find(runtime_info.propagated_literals.begin(),
                                          runtime_info.propagated_literals.end(),
                                          qbf.formula[watcher.first][other]) !=
                                runtime_info.propagated_literals.end()) {
                                runtime_info.clauseIsSat[watcher.first] = true;
                                break;
                            }

                            if (runtime_info.watchers.contains(qbf.formula[watcher.first][other]) &&
                                std::find(runtime_info.watchers[qbf.formula[watcher.first][other]].begin(),
                                          runtime_info.watchers[qbf.formula[watcher.first][other]].end(),
                                          std::pair<int, int>{watcher.first, lit_pos}) !=
                                runtime_info.watchers[qbf.formula[watcher.first][lit_pos]].end()) {

                                // If we have a conflict form unit propagation
                                if (std::find(runtime_info.propagated_literals.begin(),
                                              runtime_info.propagated_literals.end(),
                                              -qbf.formula[watcher.first][lit_pos]) !=
                                    runtime_info.propagated_literals.end()) {
                                    is_unsat = true; // if conflict found
                                    return;
                                }

                                unit_clauses.push_back(qbf.formula[watcher.first][other]);
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

    std::vector<int>
    update_watchers(runtime_info &runtimeInfo, QBF &qbf, std::vector<int> &c, std::vector<int> &cb, int i) {
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

        // Delete watchers
        for (int j = 0; j < c.size(); j++) {
            if (runtimeInfo.watchers.contains(c[j])) {
                auto newEnd = std::remove(runtimeInfo.watchers[c[j]].begin(), runtimeInfo.watchers[c[j]].end(),
                                          std::pair<int, int>{i, j});
                runtimeInfo.watchers[c[j]].erase(newEnd, runtimeInfo.watchers[c[j]].end());
            }
        }
        // Create new Watchers for the clause
        auto t = new_c.size() - 1;
        while (t > 0) {
            if (qbf.quantifierTypeIsExists[abs(new_c[t])]) {
                runtimeInfo.watchers[new_c[t]].emplace_back(i, t);
                runtimeInfo.watchers[new_c[t - 1]].emplace_back(i, t - 1);
                break;
            }
            t--;
        }
        return std::move(new_c);
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

    bool is_equal_on_exists(QBF &qbf, std::vector<int> c, std::vector<int> cb){
        for(int lit : c){
            if(qbf.quantifierTypeIsExists[abs(lit)]){
                if (std::find(cb.begin(),cb.end(), lit) == cb.end()){
                    return false;
                }
            }
        }
        return true;
    }

    void watched_literals_vivify(QBF &qbf, int global_count) {
        // create check for change
        bool change = true;
        watched_literals_unit_propagation(qbf);
        // If qbf is sat or unsat after unit propagation
        if (qbf.formula.empty() || qbf.formula[0].empty()) {
            return;
        }
        while (change) {
            change = false;

            auto runtimeInfo = create_runtime_info(qbf);
            for (int i = 0; i < qbf.formula.size(); i++) {
                // we work with a copy of the runtimeInfo. And adapt this after each step
                runtime_info qbf_tracking = runtimeInfo;

                // we want to ignore the tracking info for now.
                qbf_tracking.clauseIsSat[i] = true;

                // we take a clause
                std::vector<int> c = qbf.formula[i];

                // we take a finished clause
                std::vector<int> cb;

                bool shortened = false;

                while (!shortened && !is_equal_on_exists(qbf,c,cb)) {
                    int l = select_a_literal(qbf, c, cb);
                    std::cout << "here" << std::endl;
                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat;
                    size_t num_propagations = qbf_tracking.propagated_literals.size();
                    UP(qbf_tracking, qbf, -l, is_unsat);
                    if (std::find(qbf_tracking.clauseIsSat.begin(), qbf_tracking.clauseIsSat.end(), false) ==
                        qbf_tracking.clauseIsSat.end()) {
                        std::cout << "SAT" << std::endl;
                        qbf.formula = {};
                        return;
                    }
                    if (is_unsat) {
                        qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i);
                        if (!is_equal_on_exists(qbf,c,cb)) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for (size_t lit = num_propagations + 1; lit < qbf_tracking.propagated_literals.size(); lit++) {
                            int unit = qbf_tracking.propagated_literals[lit];

                            if (std::find(c.begin(), c.end(), unit) != c.end()) {
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i);
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
                                qbf.formula[i] = update_watchers(runtimeInfo, qbf, c, cb, i);
                                shortened = true;
                                break;
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
            }
        }
    }

}
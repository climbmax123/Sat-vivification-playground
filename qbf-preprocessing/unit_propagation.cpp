//
// Created by Christofer Held on 19.06.24.
//

#include <set>
#include "unit_propagation.h"
#include <iostream>

namespace unit {


    void watched_literals_unit_propagation_without_ur(QBF &qbf, int timeLimitInSeconds) {
        auto startTime = std::chrono::steady_clock::now();
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> unit_clauses;

        for (int i = 0; i < qbf.formula.size(); i++) {
            if (qbf.formula[i].size() > 1) {
                watchers[qbf.formula[i][qbf.formula[i].size() - 1]].emplace_back(i, qbf.formula[i].size() - 1);
                watchers[qbf.formula[i][qbf.formula[i].size() - 2]].emplace_back(i, qbf.formula[i].size() - 2);
            }
            if (qbf.formula[i].size() == 1) {
                if (!qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][0])] ||
                    std::find(unit_clauses.begin(), unit_clauses.end(), -qbf.formula[i][0]) != unit_clauses.end()) {
                    qbf.formula = {{}};
                    return;
                }
                if (std::find(unit_clauses.begin(), unit_clauses.end(), qbf.formula[i][0]) == unit_clauses.end()) {
                    unit_clauses.push_back(qbf.formula[i][0]);
                }
            }
            if (qbf.formula[i].empty()) {
                qbf.formula = {{}};
                return;
            }
        }
        if (unit_clauses.empty()) return;

        std::set<int> propagated_literals;
        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        int count = 0;

        while (count < unit_clauses.size()) {
            if (timeLimitInSeconds != -1) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - startTime).count();
                if (elapsedSeconds >= timeLimitInSeconds) {
                    std::cout << "Timeout reached ending vivify" << std::endl;
                    return;
                }
            }
            propagated_literals.insert(unit_clauses[count]);

            if (watchers.find(unit_clauses[count]) != watchers.end()) {
                for (const auto watcher: watchers[unit_clauses[count]]) {
                    sat_clauses[watcher.first] = true;
                }
                watchers.erase(unit_clauses[count]);
            }

            if (watchers.find(-unit_clauses[count]) != watchers.end()) {
                // we have to replace the watchers for that literal
                for (const auto watcher: watchers[-unit_clauses[count]]) {
                    int clause = watcher.first;

                    if (sat_clauses[watcher.first]) continue;

                    int lit_pos = watcher.second - 1;
                    bool found_new_watcher = false;
                    int other_watcher_pos = watcher.second + 1;

                    while (lit_pos >= 0) {
                        // if next position is already a watcher
                        if (watchers.find(qbf.formula[clause][lit_pos]) != watchers.end() &&
                            std::find(watchers[qbf.formula[clause][lit_pos]].begin(),
                                      watchers[qbf.formula[clause][lit_pos]].end(),
                                      std::pair<int, int>{clause, lit_pos}) !=
                            watchers[qbf.formula[clause][lit_pos]].end()) {
                            other_watcher_pos = lit_pos; // we store that to speed up search
                            lit_pos--;
                            continue;
                        }
                        // if already the positive is propagated we can set the clause true and exit
                        if (propagated_literals.find(qbf.formula[clause][lit_pos]) != propagated_literals.end()) {
                            sat_clauses[clause] = true;
                            break;
                        }

                        // if already the negative is propagated continue
                        if (propagated_literals.find(-qbf.formula[clause][lit_pos]) != propagated_literals.end()) {
                            lit_pos--;
                            continue;
                        }

                        // the literal is not watched, the literal is not propagated so we can use it for the
                        // new watched literal
                        found_new_watcher = true;

                        if (watchers.find(qbf.formula[clause][lit_pos]) != watchers.end()) {
                            watchers[qbf.formula[clause][lit_pos]].emplace_back(clause, lit_pos);
                            break;
                        }

                        watchers[qbf.formula[clause][lit_pos]] = {{clause, lit_pos}};
                        break;
                    }
                    if (!found_new_watcher && !sat_clauses[clause]) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other < qbf.formula[clause].size(); other++) {
                            if (propagated_literals.find(qbf.formula[clause][other]) != propagated_literals.end()) {
                                sat_clauses[clause] = true;
                                break;
                            }

                            if (watchers.find(qbf.formula[clause][other]) != watchers.end() &&
                                std::find(watchers[qbf.formula[clause][other]].begin(),
                                          watchers[qbf.formula[clause][other]].end(),
                                          std::pair<int, int>{clause, other}) !=
                                watchers[qbf.formula[clause][other]].end()) {

                                if (std::find(unit_clauses.begin(), unit_clauses.end(), -qbf.formula[clause][other]) !=
                                    unit_clauses.end()) {
                                    std::cout << "found conflict1" << std::endl;
                                    qbf.formula = {{}}; // if conflict found
                                    return;
                                }

                                if (!qbf.quantifierTypeIsExists[std::abs(qbf.formula[clause][other])]) {
                                    std::cout << "found conflict 2" << clause << ", " << other << " - "
                                              << std::abs(qbf.formula[clause][other]) << std::endl;
                                    qbf.formula = {{}};
                                    return;
                                }

                                unit_clauses.push_back(qbf.formula[clause][other]);
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
            for (int j: qbf.formula[i]) {
                if (propagated_literals.find(j) != propagated_literals.end()) {
                    add = false;
                    break;
                }
                if (propagated_literals.find(-j) != propagated_literals.end()) {
                    continue;
                }
                cl.push_back(j);
            }
            if (add) {
                new_formula.push_back(std::move(cl));
            }
        }

        std::vector<int> newQuantifierOrder;
        for (int element: qbf.quantifierOrder) {
            bool found = false;
            for (const auto &sub_vector: new_formula) {
                if (std::find(sub_vector.begin(), sub_vector.end(), element) != sub_vector.end() ||
                    std::find(sub_vector.begin(), sub_vector.end(), -element) != sub_vector.end()) {
                    found = true;
                    break;
                }
            }
            if (found) {
                newQuantifierOrder.push_back(element);
            }
        }

        qbf.formula = std::move(new_formula);
        qbf.quantifierOrder = std::move(newQuantifierOrder);
    }


    void prepare_watchers(QBF &qbf, std::unordered_map<int, std::vector<std::pair<int, int>>> &watchers,
                          std::vector<int> &unit_clauses) {
        for (int i = 0; i < qbf.formula.size(); i++) {
            auto t = qbf.formula[i].size() - 1;
            while (t > 0) {
                if (qbf.quantifierTypeIsExists[abs(qbf.formula[i][t])]) {
                    if (t >= 1) {
                        watchers[qbf.formula[i][t]].emplace_back(i, t);
                        watchers[qbf.formula[i][t - 1]].emplace_back(i, t - 1);
                        break;
                    }
                }
                t--;
            }
            // check if this is clause ist unit.
            if (t == 0) {
                auto b = qbf.formula[i].size() - 1;
                while (!qbf.quantifierTypeIsExists[abs(qbf.formula[i][b])]) {
                    qbf.formula[i].erase(qbf.formula[i].end() - 1);
                    b--;
                }
                if (qbf.formula[i].size() == 1) {
                    unit_clauses.push_back(qbf.formula[i][0]);
                    continue;
                }
            }
        }
    }


    void find_other_watcher(QBF &qbf, std::unordered_map<int, std::vector<std::pair<int, int>>> &watchers,
                            const std::pair<int, int> &watcher, int &other_watcher_pos,
                            std::set<int> &propagated_literals,
                            std::vector<bool> &sat_clauses) {


        // We find the next literal and check if on the pos it is true that should contain all literals.
        if (!qbf.quantifierTypeIsExists[abs(qbf.formula[watcher.first][watcher.second])]) {
            for (; other_watcher_pos < qbf.formula[watcher.first].size(); other_watcher_pos++) {
                if (propagated_literals.find(qbf.formula[watcher.first][other_watcher_pos]) != propagated_literals.end()) {
                    sat_clauses[watcher.first] = true;
                }

                if (watchers.find(qbf.formula[watcher.first][other_watcher_pos]) != watchers.end() &&
                    std::find(watchers[qbf.formula[watcher.first][other_watcher_pos]].begin(),
                              watchers[qbf.formula[watcher.first][other_watcher_pos]].end(),
                              std::pair<int, int>{watcher.first, other_watcher_pos}) !=
                    watchers[qbf.formula[watcher.first][other_watcher_pos]].end()) {
                    break;
                }
            }
        } else {
            int pos = other_watcher_pos;
            bool found = false;
            for (; pos < qbf.formula[watcher.first].size(); pos++) {
                if (propagated_literals.find(qbf.formula[watcher.first][pos]) != propagated_literals.end()) {
                    sat_clauses[watcher.first] = true;
                }
                if (watchers.find(qbf.formula[watcher.first][pos]) != watchers.end() &&
                    std::find(watchers[qbf.formula[watcher.first][pos]].begin(),
                              watchers[qbf.formula[watcher.first][pos]].end(),
                              std::pair<int, int>{watcher.first, pos}) !=
                    watchers[qbf.formula[watcher.first][pos]].end()) {
                    other_watcher_pos = pos;
                    found = true;
                    break;
                }
            }
            if (!found) {
                pos = other_watcher_pos - 2;
                for (; pos >= 0; pos--) {
                    if (propagated_literals.find(qbf.formula[watcher.first][pos]) != propagated_literals.end()) {
                        sat_clauses[watcher.first] = true;
                    }
                    if (watchers.find(qbf.formula[watcher.first][pos]) != watchers.end()&&
                        std::find(watchers[qbf.formula[watcher.first][pos]].begin(),
                                  watchers[qbf.formula[watcher.first][pos]].end(),
                                  std::pair<int, int>{watcher.first, pos}) !=
                        watchers[qbf.formula[watcher.first][pos]].end()) {
                        other_watcher_pos = pos;
                        break;
                    }
                }
            }
        }
    }


    void watched_literals_unit_propagation_with_ur(QBF &qbf, int timeLimitInSeconds) {
        auto startTime = std::chrono::steady_clock::now();
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers; // (clause, lit)

        std::vector<int> unit_clauses;

        // Step 1: Prepare The watchers
        prepare_watchers(qbf, watchers, unit_clauses);

        // We keep track and apply later
        std::set<int> propagated_literals;
        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        int count = 0;
        while (count < unit_clauses.size()) {
            if (timeLimitInSeconds != -1) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - startTime).count();
                if (elapsedSeconds >= timeLimitInSeconds) {
                    std::cout << "Timeout reached ending vivify" << std::endl;
                    break;
                }
            }
            propagated_literals.insert(unit_clauses[count]);
            // Step2: We remove alle literals who are set true (We only propagate existential so issue with universals)
            if (watchers.find(unit_clauses[count]) != watchers.end())  {
                for (int w = 0; w < watchers[unit_clauses[count]].size(); w++) {
                    sat_clauses[watchers[unit_clauses[count]][w].first] = true;
                }
                watchers.erase(unit_clauses[count]);
            }

            // Step3: The negative case: we have to update all watchers that watch -l and reset them
            if (watchers.find(-unit_clauses[count]) != watchers.end()) {

                for (int w = 0; w < watchers[-unit_clauses[count]].size(); w++) {

                    auto wt = watchers[-unit_clauses[count]][w];
                    // If-clause is sat we don't care
                    if (sat_clauses[watchers[-unit_clauses[count]][w].first]) continue;

                    // Step 3.1 Find the other watcher
                    int other_watcher_pos = watchers[-unit_clauses[count]][w].second + 1;
                    find_other_watcher(qbf, watchers, watchers[-unit_clauses[count]][w], other_watcher_pos,
                                       propagated_literals, sat_clauses);
                    if (sat_clauses[watchers[-unit_clauses[count]][w].first]) continue; // could be found to be sat in while the search of the other watcher

                    // Step 3.2 Find next watched position
                    int found_new_watcher = 0;

                    // We can assume if the watcher pos < the other watcher then we will overstep it.
                    // Therefore we look if the other watcher is < the current otherwise it is existential by design
                    // If it is existential we look other watcher pos-1 for the next exist/forall literal
                    // OTW we set the other watcher to the next existential var and thenn start other_watcher - 1 with the search.
                    // We have to check if we are already sat all the time.
                    if (watchers[-unit_clauses[count]][w].second > other_watcher_pos &&
                        !qbf.quantifierTypeIsExists[std::abs(
                                qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos])]) {

                        watchers[qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]].erase(
                                std::find(
                                        watchers[qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]].begin(),
                                        watchers[qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]].end(),
                                        std::pair<int, int>{watchers[-unit_clauses[count]][w].first,
                                                            other_watcher_pos}));

                        while (other_watcher_pos > 0) {
                            other_watcher_pos--;
                            if (propagated_literals.find(
                                    qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]) != propagated_literals.end()) {
                                sat_clauses[watchers[-unit_clauses[count]][w].first] = true;
                                break;
                            }
                            if (propagated_literals.find(
                                    -qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]) != propagated_literals.end()) {
                                continue;
                            }
                            if (!qbf.quantifierTypeIsExists[std::abs(
                                    qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos])]) {
                                continue;
                            }
                            found_new_watcher++;
                            break;
                        }
                    } else {
                        found_new_watcher++;
                    }
                    if (sat_clauses[watchers[-unit_clauses[count]][w].first]) continue;

                    for (int watcher_pos = std::min(watchers[-unit_clauses[count]][w].second - 1, other_watcher_pos-1);
                         watcher_pos >= 0; watcher_pos--) {
                        if (propagated_literals.find(
                                qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]) != propagated_literals.end()) {
                            sat_clauses[watchers[-unit_clauses[count]][w].first] = true;
                            break;
                        }
                        if (propagated_literals.find(
                                -qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]) != propagated_literals.end()) {
                            sat_clauses[watchers[-unit_clauses[count]][w].first] = true;
                            continue;
                        }
                        found_new_watcher++;
                        watchers[qbf.formula[watchers[-unit_clauses[count]][w].first][watcher_pos]].emplace_back(
                                watchers[-unit_clauses[count]][w].first, watcher_pos);
                        break;
                    }
                    if (sat_clauses[watchers[-unit_clauses[count]][w].first]) continue;
                    if (found_new_watcher == 2) {

                        watchers[qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]].emplace_back(
                                watchers[-unit_clauses[count]][w].first,
                                other_watcher_pos);
                    }
                    if (found_new_watcher == 1) {
                        unit_clauses.push_back(qbf.formula[watchers[-unit_clauses[count]][w].first][other_watcher_pos]);
                    }

                    if (found_new_watcher == 0) {
                        // formula is unsat
                        qbf.formula = {{}};
                        return;
                    }
                }
            }
            count++;
        }
        std::vector<std::vector<int>> new_formula;

        // UP apply happens here
        for (int i = 0; i < qbf.formula.size(); i++) {
            if (sat_clauses[i]) continue;
            std::vector<int> cl;
            bool add = true;
            bool reduce = true;
            for (int j = qbf.formula[i].size() - 1; j >= 0; j--) {
                if (reduce && !qbf.quantifierTypeIsExists[abs(qbf.formula[i][j])]) {
                    continue;
                }
                if (propagated_literals.find(qbf.formula[i][j]) != propagated_literals.end()) {
                    add = false;
                    break;
                }
                if (propagated_literals.find(-qbf.formula[i][j]) != propagated_literals.end()) {
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
                if (std::find(sub_vector.begin(), sub_vector.end(), element) != sub_vector.end() ||
                    std::find(sub_vector.begin(), sub_vector.end(), -element) != sub_vector.end()) {
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

}
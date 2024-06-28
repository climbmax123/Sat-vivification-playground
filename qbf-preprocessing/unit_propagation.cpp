//
// Created by Christofer Held on 19.06.24.
//

#include <set>
#include "unit_propagation.h"
#include <iostream>

namespace unit {

/*
void watched_literals_unit_propagation(QBF &qbf) {
    std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
    std::vector<int> unit_clauses;

    for (int i = 0; i < cnf.size(); i++) {
        if (cnf[i].size() > 1) {
            watchers[cnf[i][cnf[i].size() - 1]].emplace_back(i, cnf[i].size() - 1);
            watchers[cnf[i][cnf[i].size() - 2]].emplace_back(i, cnf[i].size() - 2);
        }
        if (cnf[i].size() == 1) {
            if (std::find(unit_clauses.begin(), unit_clauses.end(), -cnf[i][0]) != unit_clauses.end()) {
                cnf = {{}};
                return;
            }
            if (std::find(unit_clauses.begin(), unit_clauses.end(), cnf[i][0]) == unit_clauses.end()) {
                unit_clauses.push_back(cnf[i][0]);
            }
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

                int lit_pos = watcher.second - 1;
                bool found_new_watcher = false;
                int other_watcher_pos = watcher.second + 1;

                while (lit_pos >= 0) {
                    // if next position is already a watcher
                    if (watchers.contains(cnf[clause][lit_pos]) &&
                        std::find(watchers[cnf[clause][lit_pos]].begin(),
                                  watchers[cnf[clause][lit_pos]].end(),
                                  std::pair<int, int>{clause, lit_pos}) !=
                        watchers[cnf[clause][lit_pos]].end()) {
                        other_watcher_pos = lit_pos; // we store that to speed up search
                        lit_pos--;
                        continue;
                    }
                    // if already the positive is propagated we can set the clause true and exit
                    if (propagated_literals.contains(cnf[clause][lit_pos])) {
                        sat_clauses[clause] = true;
                        break;
                    }

                    // if already the negative is propagated continue
                    if (propagated_literals.contains(-cnf[clause][lit_pos])) {
                        lit_pos--;
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
                if (!found_new_watcher && !sat_clauses[clause]) {
                    // if we only have one watcher we are done
                    for (int other = other_watcher_pos; other < cnf[clause].size(); other++) {
                        if (propagated_literals.contains(cnf[clause][other])) {
                            sat_clauses[clause] = true;
                            break;
                        }

                        if (watchers.contains(cnf[clause][other]) &&
                            std::find(watchers[cnf[clause][other]].begin(),
                                      watchers[cnf[clause][other]].end(),
                                      std::pair<int, int>{clause, other}) !=
                            watchers[cnf[clause][other]].end()) {

                            if (std::find(unit_clauses.begin(), unit_clauses.end(), -cnf[clause][other]) !=
                                unit_clauses.end()) {
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
 */

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
                if (propagated_literals.contains(qbf.formula[watcher.first][other_watcher_pos])) {
                    sat_clauses[watcher.first] = true;
                }

                if (watchers.contains(qbf.formula[watcher.first][other_watcher_pos]) &&
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
                if (propagated_literals.contains(qbf.formula[watcher.first][pos])) {
                    sat_clauses[watcher.first] = true;
                }
                if (watchers.contains(qbf.formula[watcher.first][pos]) &&
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
                    if (propagated_literals.contains(qbf.formula[watcher.first][pos])) {
                        sat_clauses[watcher.first] = true;
                    }
                    if (watchers.contains(qbf.formula[watcher.first][pos]) &&
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


    void watched_literals_unit_propagation_with_up(QBF &qbf) {

        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers; // (clause, lit)

        std::vector<int> unit_clauses;

        // Step 1: Prepare The watchers
        prepare_watchers(qbf, watchers, unit_clauses);

        // We keep track and apply later
        std::set<int> propagated_literals;
        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        int count = 0;
        while (count < unit_clauses.size()) {
            propagated_literals.insert(unit_clauses[count]);
            std::cout << "Propagating: " << unit_clauses[count] << std::endl;
            // Step2: We remove alle literals who are set true (We only propagate existential so issue with universals)
            if (watchers.contains(unit_clauses[count])) {
                for (const auto watcher: watchers[unit_clauses[count]]) {
                    sat_clauses[watcher.first] = true;
                }
            }
/*
            std::cout << "Formula: ";
            for(int i = 0; i < qbf.formula.size(); i++){
                if(sat_clauses[i]){
                    std::cout << "T ";
                } else {
                    std::cout << "F ";
                }
                std::cout <<"(";
                for(auto lit:qbf.formula[i])
                    std::cout << lit << ",";
                std::cout << ") ";
            }
            */
            std::cout << std::endl;

            // Step3: The negative case: we have to update all watchers that watch -l and reset them
            if (watchers.contains(-unit_clauses[count])) {
                for (const auto watcher: watchers[-unit_clauses[count]]) {

                    // If-clause is sat we don't care
                    if (sat_clauses[watcher.first]) continue;

                    // Step 3.1 Find the other watcher
                    int other_watcher_pos = watcher.second + 1;
                    find_other_watcher(qbf, watchers, watcher, other_watcher_pos, propagated_literals, sat_clauses);

                    if (sat_clauses[watcher.first]) continue; // could be found to be sat in while the search of the other watcher

                    // Step 3.2 Find next watched position
                    int found_new_watcher = 0;

                    // We can assume if the watcher pos < the other watcher then we will overstep it.
                    // Therefore we look if the other watcher is < the current otherwise it is existential by design
                    // If it is existential we look other watcher pos-1 for the next exist/forall literal
                    // OTW we set the other watcher to the next existential var and thenn start other_watcher - 1 with the search.
                    std::cout << "here" << std::endl;
                    // We have to check if we are already sat all the time.
                    if (watcher.second > other_watcher_pos) {
                        if (!qbf.quantifierTypeIsExists[std::abs(qbf.formula[watcher.first][other_watcher_pos])]) {
                            watchers[qbf.formula[watcher.first][other_watcher_pos]].erase(
                                    std::find(watchers[qbf.formula[watcher.first][other_watcher_pos]].begin(),
                                              watchers[qbf.formula[watcher.first][other_watcher_pos]].end(),
                                              std::pair<int, int>{watcher.first, other_watcher_pos}));

                            while (other_watcher_pos > 0) {
                                other_watcher_pos--;
                                if (propagated_literals.contains(qbf.formula[watcher.first][other_watcher_pos])) {
                                    sat_clauses[watcher.first] = true;
                                    break;
                                }
                                if (propagated_literals.contains(-qbf.formula[watcher.first][other_watcher_pos])) {
                                    sat_clauses[watcher.first] = true;
                                    continue;
                                }
                                if (!qbf.quantifierTypeIsExists[std::abs(
                                        qbf.formula[watcher.first][other_watcher_pos])]) {
                                    continue;
                                }
                                found_new_watcher++;
                                break;
                            }
                        }
                    }
                    if (sat_clauses[watcher.first]) continue;

                    for (int watcher_pos = other_watcher_pos - 1; watcher_pos >= 0; watcher_pos--) {
                        if (propagated_literals.contains(qbf.formula[watcher.first][other_watcher_pos])) {
                            sat_clauses[watcher.first] = true;
                            break;
                        }
                        if (propagated_literals.contains(-qbf.formula[watcher.first][other_watcher_pos])) {
                            sat_clauses[watcher.first] = true;
                            continue;
                        }
                        found_new_watcher++;
                        watchers[qbf.formula[watcher.first][watcher_pos]].emplace_back(watcher.first, watcher_pos);
                        break;
                    }
                    std::cout << "here " << watcher.first << ", " << found_new_watcher << std::endl;
                    if (sat_clauses[watcher.first]) continue;
                    std::cout << "here " << watcher.first << ", " << found_new_watcher << std::endl;
                    if (found_new_watcher == 2) {

                        watchers[qbf.formula[watcher.first][other_watcher_pos]].emplace_back(watcher.first,
                                                                                             other_watcher_pos);
                    }

                    if (found_new_watcher == 1) {
                        unit_clauses.push_back(qbf.formula[watcher.first][other_watcher_pos]);
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
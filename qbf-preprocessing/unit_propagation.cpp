//
// Created by Christofer Held on 19.06.24.
//

#include <set>
#include "unit_propagation.h"


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

void watched_literal_universal_reduction(QBF &qbf) {

}



void watched_literals_unit_propagation_with_universal_reduction(QBF &qbf) {

    std::unordered_map<int, QuantifierType> quantifiers;
    std::unordered_map<int, std::vector<std::pair<int, int>>> watchers; // (clause, lit)

    std::vector<int> unit_clauses;

    // ----------------------------------------
    // Prepare Watched literals
    // ----------------------------------------
    for (int i = 0; i < qbf.formula.size(); i++) {
        auto t = qbf.formula[i].size() - 1;
        while (t > 0) {
            if (qbf.quantifierType[abs(qbf.formula[i][t])] == EXISTS) {
                if(t > 1) {
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
            while (qbf.quantifierType[abs(qbf.formula[i][b])] == FORALL) {
                qbf.formula[i].erase(qbf.formula[i].end() - 1);
                b--;
            }
            if(qbf.formula[i].size() == 1) {
                unit_clauses.push_back(qbf.formula[i][0]);
                continue;
            }
            qbf.formula = {{}};
            return;
        }
    }

    // we keep track and later we use that to reduce the formula
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
                int other_watcher_pos = watcher.second + 1;

                if(qbf.quantifierType[abs(qbf.formula[watcher.first][watcher.second])] == FORALL){
                    for(; other_watcher_pos < qbf.formula[watcher.first].size(); other_watcher_pos++){
                        if (watchers.contains(qbf.formula[watcher.first][other_watcher_pos]) &&
                            std::find(watchers[qbf.formula[watcher.first][other_watcher_pos]].begin(),
                                      watchers[qbf.formula[watcher.first][other_watcher_pos]].end(),
                                      std::pair<int, int>{watcher.first, other_watcher_pos}) !=
                            watchers[qbf.formula[watcher.first][other_watcher_pos]].end()) {
                            break;
                        }
                    }
                }

                if()

                bool found_new_watcher = false;

                 // we want to find the other watcher quickly.

                // We go the other way around from current pos to 0
                while (lit_pos >= 0) {

                    // if next position is already the other Watcher
                    if (watchers.contains(qbf.formula[watcher.first][lit_pos]) &&
                        std::find(watchers[qbf.formula[watcher.first][lit_pos]].begin(),
                                  watchers[qbf.formula[watcher.first][lit_pos]].end(),
                                  std::pair<int, int>{watcher.first, lit_pos}) !=
                        watchers[qbf.formula[watcher.first][lit_pos]].end()) {

                        // The other Watcher must be existential if we overstep it. Otw. we have to rest it
                        if (qbf.quantifierType[abs(qbf.formula[watcher.first][lit_pos])] == FORALL) {
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
                                if (qbf.quantifierType[abs(qbf.formula[watcher.first][lit_pos])] == FORALL) {
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
                        && qbf.quantifierType[abs(qbf.formula[watcher.first][lit_pos])] == EXISTS) {
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
            if (reduce && qbf.quantifierType[abs(qbf.formula[i][j])] == FORALL) {
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
    std::unordered_map<int, QuantifierType> newQuantors;
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
            newQuantors[element] = qbf.quantifierType[abs(element)];
        }
    }

    qbf.formula = std::move(new_formula);
    qbf.quantifierType = std::move(newQuantors);
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
            if (qbf.quantifierType[abs(qbf.formula[i][t])] == EXISTS) {
                info.watchers[qbf.formula[i][t]].emplace_back(i, t);
                info.watchers[qbf.formula[i][t - 1]].emplace_back(i, t - 1);
                break;
            }
            t--;
        }
    }
    return std::move(info);
}
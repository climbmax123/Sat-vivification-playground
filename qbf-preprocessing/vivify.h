//
// Created by christofer on 26.04.24.
//

#ifndef VIVIFCATION_VIVIFY_H
#define VIVIFCATION_VIVIFY_H

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



#endif //VIVIFCATION_VIVIFY_H

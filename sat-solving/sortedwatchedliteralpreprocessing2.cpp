//
// Created by Christofer Held on 10.06.24.
//

#include "sortedwatchedliteralpreprocessing2.h"


//
// Created by Christofer Held on 21.05.24.
//

#include "sortedwatchedliteralpreprocessing.h"

#include <algorithm>
#include <chrono>
#include <map>

namespace sorted2 {

    void watched_literals_unit_propagation(CDNF_formula &cnf) {
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


    std::unordered_map<int, std::vector<std::pair<int, int>>> create_watched_literal_mapping(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        for (int i = 0; i < cnf.size(); i++) {
            for (int j = 0; j < 2; j++) {
                watchers[cnf[i][cnf[i].size() - 1]].emplace_back(i, cnf[i].size() - 1);
                watchers[cnf[i][cnf[i].size() - 2]].emplace_back(i, cnf[i].size() - 2);
            }
        }
        return std::move(watchers);
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

                    int lit_pos = watcher.second - 1;
                    bool found_new_watcher = false;
                    int other_watcher_pos = watcher.second + 1;

                    while (lit_pos >= 0) {
                        // if next position is already a watcher
                        if (runtime_info.watchers.contains(cnf[clause][lit_pos]) &&
                            std::find(runtime_info.watchers[cnf[clause][lit_pos]].begin(),
                                      runtime_info.watchers[cnf[clause][lit_pos]].end(),
                                      std::pair<int, int>{clause, lit_pos}) !=
                            runtime_info.watchers[cnf[clause][lit_pos]].end()) {
                            other_watcher_pos = lit_pos; // we store that to speed up search
                            lit_pos--;
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
                            lit_pos--;
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
                    if (!found_new_watcher && !runtime_info.clauseIsSat[clause]) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other < cnf[clause].size(); other++) {

                            if (std::find(runtime_info.propagated_literals.begin(),
                                          runtime_info.propagated_literals.end(),
                                          cnf[clause][other]) != runtime_info.propagated_literals.end()) {
                                runtime_info.clauseIsSat[clause] = true;
                                break;
                            }

                            if (runtime_info.watchers.contains(cnf[clause][other]) &&
                                std::find(runtime_info.watchers[cnf[clause][other]].begin(),
                                          runtime_info.watchers[cnf[clause][other]].end(),
                                          std::pair<int, int>{clause, other}) !=
                                runtime_info.watchers[cnf[clause][other]].end()) {

                                if (std::find(unit_clauses.begin(),
                                              unit_clauses.end(),
                                              -cnf[clause][other]) != unit_clauses.end()) {
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


//
void sort_cnf(CDNF_formula &cnf, std::map<int, int> &mapping, int offset) {
    // We know it cannot contain pures anymore
    auto compare = [&mapping](int a, int b) {
        auto min_a = std::min(mapping[a], mapping[-a]);
        auto min_b = std::min(mapping[b], mapping[-b]);
        if (min_a != min_b) {
            return min_a > min_b;
        }
        auto max_a = std::max(mapping[a], mapping[-a]);
        auto max_b = std::max(mapping[b], mapping[-b]);
        return max_a < max_b;
    };

    for (int i = offset; i < cnf.size(); i++) {
        std::sort(cnf[i].begin(), cnf[i].end(), compare);
    }

    auto compare2 = [&mapping](const std::vector<int> &a, const std::vector<int> &b) {
        auto get_min_size = [&mapping](int val) {
            return std::min(mapping[val], mapping[-val]);
        };


        int a_c = a.size() - 1;
        int b_c = b.size() - 1;

        while (a_c >= 0 && b_c >= 0) {
            int min_a_last = get_min_size(a[a_c]);
            int min_b_last = get_min_size(b[b_c]);
            if (min_a_last != min_b_last) {
                return min_a_last < min_b_last; // Aufsteigend sortieren
            }
            a_c--;
            b_c--;
        }

        if (a_c < 0) {
            return false;
        } else {
            return true;
        }
    };

    std::sort(cnf.begin() + offset, cnf.end(), compare2);
}


std::map<int, int> creat_mapping(CDNF_formula &formula) {
    std::map<int, int> map;
    for (const auto &cl: formula) {
        for (int i: cl) {
            if (map.contains(i)) {
                map[i]++;
            } else {
                map.insert({i, 1});
            }
        }
    }
    return std::move(map);
}

void pureLiteralElimination(CDNF_formula &formula, std::map<int, int> &mapping) {
    std::vector<bool> sat_clauses = std::vector<bool>(formula.size());

    std::set<int> pures = {};
    for (auto literal: mapping) {
        if (mapping.contains(-literal.first) && mapping[literal.first] > 0) {
            continue;
        }
        pures.insert(literal.first);
    }

    while (!pures.empty()) {
        std::set<int> new_pures = {};
        for (size_t i = 0; i < formula.size(); i++) {
            if (sat_clauses[i]) { continue; }
            for (int literal: formula[i]) {
                if (pures.find(literal) != pures.end()) {
                    sat_clauses[i] = true;
                    for (int l: formula[i]) {
                        mapping[l]--;
                        if(mapping[l] == 0 && mapping[-l] > 0){
                            new_pures.insert(-l);
                        }
                    }
                }
            }
        }
        pures = new_pures;
    }

    CDNF_formula newFormula;
    for (int i = 0; i < formula.size(); i++) {
        if (!sat_clauses[i]) {
            newFormula.push_back(std::move(formula[i]));
        }
    }

    formula = std::move(newFormula);
}


int pureRuntimeLiteralElimination(CDNF_formula &formula, std::map<int, int> &mapping,
                                  std::set<int> pures, int &found_pures, int &removed_clauses, int step) {

    if(pures.empty()) return 0;

    std::vector<bool> sat_clauses = std::vector<bool>(formula.size());

    while (!pures.empty()) {
        std::set<int> new_pures = {};
        for (size_t i = 0; i < formula.size(); i++) {
            if (sat_clauses[i]) { continue; }
            for (int literal: formula[i]) {
                if (pures.find(literal) != pures.end()) {
                    sat_clauses[i] = true;
                    for (int l: formula[i]) {
                        mapping[l]--;
                        if(mapping[l] == 0 && mapping[-l] > 0){
                            new_pures.insert(-l);
                        }
                    }
                }
            }
        }
        pures = new_pures;
    }

    CDNF_formula newFormula;
    int count = 0;
    for (int i = 0; i < formula.size(); i++) {
        if (!sat_clauses[i]) {
            newFormula.push_back(std::move(formula[i]));
        }
        if(i <= step){
            count++;
        }
    }

    formula = std::move(newFormula);
    return count;
}


int runtimeUnitPropagation(CDNF_formula &cnf, std::map<int, int> &mapping,
                           int new_unit, int &removed_clauses, int &removed_literals, int step) {

    std::vector<int> unit_clauses = {new_unit};

    std::set<int> propagated_literals;

    int sum_val = 0;
    int count = 0;
    while (count < unit_clauses.size()) {
        int lit = unit_clauses[count];
        CDNF_formula new_cnf;
        int s_count = 0;
        for (auto &clause: cnf) {
            if (std::find(clause.begin(), clause.end(), lit) != clause.end()) {
                if(count <= step) {
                    sum_val++;
                }
                removed_clauses++;
                for (auto i: clause) {
                    mapping[i]--;
                }
            } else if (std::find(clause.begin(), clause.end(), -lit) != clause.end()) {
                removed_literals++;
                // Delete the Literal
                clause.erase(std::find(clause.begin(), clause.end(), -lit));
                mapping[-lit]--;

                // Create new Watchers.
                if (clause.size() == 1) {
                    unit_clauses.push_back(clause[0]);
                }
                new_cnf.push_back(std::move(clause));
            } else {
                new_cnf.push_back(std::move(clause));
            }
            s_count++;
        }
        count++;
        cnf = std::move(new_cnf);
    }

    return sum_val;
}


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

void vivify_with_sorted_pure_lit(CDNF_formula &cnf, int timeLimitInSeconds) {

    // We want to track the runtime to stop running after timeout.
    auto startTime = std::chrono::steady_clock::now();


    std::cout << "Started Preparing Formula for Vivify: " << std::endl;

    // remove unit clauses:
    auto start_unit_el_time = high_resolution_clock::now();
    auto cnf_clauses = cnf.size();
    watched_literals_unit_propagation(cnf);
    auto time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_unit_el_time).count();
    std::cout << "\t UP. \t\t\t\t Time Consumed: " << std::to_string(time_needed) << " ms" << "\t\t Reduced Clauses: "
              << cnf_clauses - cnf.size() << std::endl;

    // create mapping for on the fly pure elimination
    auto start_mapping_time = high_resolution_clock::now();
    std::map<int, int> mapping = creat_mapping(cnf);
    time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_mapping_time).count();
    std::cout << "\t Created mapping. \t Time Consumed: " << std::to_string(time_needed) << " ms" << std::endl;

    // do pure literal elimination
    auto start_pure_el_time = high_resolution_clock::now();
    cnf_clauses = cnf.size();
    pureLiteralElimination(cnf, mapping);
    time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_pure_el_time).count();
    std::cout << "\t Pure E.\t\t\t Time Consumed: " << std::to_string(time_needed) << " ms" << "\t\t Reduced Clauses: "
              << cnf_clauses - cnf.size() << std::endl;

    /*
    // sort the formula
    auto start_sort_time = high_resolution_clock::now();
    sort_cnf(cnf, mapping, 0);
    time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_sort_time).count();
    std::cout << "\t Sorting \t\t\t Time Consumed: " << std::to_string(time_needed) << " ms" << std::endl;
*/
    // create the watchers
    auto start_watcher_create_time = high_resolution_clock::now();
    auto watchers = create_watched_literal_mapping(cnf);
    time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_watcher_create_time).count();
    std::cout << "\t Watcher Created \t Time Consumed: " << std::to_string(time_needed) << " ms\n" << std::endl;

    // start vivify
    std::cout << "Start vivify loop:" << std::endl;
    // Preprocess Runtime Metrics
    int removed_clauses = 0;
    int removed_literals = 0;
    int found_pures = 0;

    bool change = true;

    while (change) {
        change = false;

        int step = 0;
        while (step < cnf.size()) {
            if(step % static_cast<int>(cnf.size()/4) == 0){

                auto start_sort_time = high_resolution_clock::now();
                sort_cnf(cnf, mapping, step);

                watchers = create_watched_literal_mapping(cnf);
                time_needed = duration_cast<milliseconds>(high_resolution_clock::now() - start_sort_time).count();
                std::cout << "\t Sorting: \t\t\t Time Consumed: " << std::to_string(time_needed) << " ms" << std::endl;
            }

            if (step % 100 == 0){
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - startTime).count();
                std::cout << "-> " << step << ":" << "rm clauses: " << removed_clauses << ",\t rm lits: " << removed_literals
                          << ",\t found pures: " << found_pures << ",\t time elapsed: " << elapsedSeconds << "s" << std::endl;
            }

            // If already sat or unsat
            if (cnf.empty()) return;
            if (cnf.size() == 1 && cnf[0].empty()) return;

            // check if timeout
            if (timeLimitInSeconds != -1) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - startTime).count();
                if (elapsedSeconds >= timeLimitInSeconds) {
                    std::cout << "Timeout reached ending vivify" << std::endl;
                    return;
                }
            }

            // we now work with the tracking info no need to create
            runtime_info cnf_tracking = create_runtime_info(cnf, watchers);
            cnf_tracking.clauseIsSat[step] = true;

            // we take a clause
            std::vector<int> c = cnf[step];

            // we take a finished clause
            std::vector<int> cb;
            bool shortened = false;

            int pos = 0;
            while (!shortened && c != cb && cb.size() < c.size()) {
                int l = c[pos]; // it is sorted... we want to go from 0 to end
                pos++;
                cb.push_back(l);
                // we work with the unit_tracking. It returns uns everything we need to know.
                bool is_unsat = false;
                size_t num_propagations = cnf_tracking.propagated_literals.size();
                UP(cnf_tracking, cnf, -l, is_unsat);
                if (is_unsat) {
                    if (c != cb) {
                        cnf[step] = std::move(cb);
                        std::set<int> pures;

                        // Update mapping
                        for (size_t i = cnf[step].size(); i < c.size(); i++) {
                            mapping[c[i]]--;
                            removed_literals++;
                            if (mapping[c[i]] == 0 && mapping.contains(-c[i]) && mapping[-c[i]] > 0) {
                                pures.insert(-c[i]);
                                found_pures++;
                            }
                        }

                        // We can assume that if we don't create new pures it does not contain any.
                        if (!pures.empty()) {
                            step -= pureRuntimeLiteralElimination(cnf, mapping, std::move(pures),
                                                                  found_pures,
                                                                  removed_clauses, step);
                            watchers = create_watched_literal_mapping(cnf);
                        }

                        shortened = true;

                        // found new unit
                        if (cnf[step].size() == 1) {
                            // We do that with UP
                            step -= runtimeUnitPropagation(cnf, mapping, cnf[step][0], removed_clauses,
                                                           removed_literals, step);
                            watchers = create_watched_literal_mapping(cnf);
                        }
                    } else {
                        printCNF({cb});
                        cnf[step] = std::move(cb);
                    }
                    step++;

                } else {
                    if (num_propagations + 1 < cnf_tracking.propagated_literals.size()) {
                        // find if exists a positive literal
                        auto lit_pos_c = std::find_if(
                                cnf_tracking.propagated_literals.begin() + num_propagations + 1,
                                cnf_tracking.propagated_literals.end(), [c](const int lit) {
                                    return std::find(c.begin(), c.end(), lit) != c.end();
                                });
                        // then we want to take that case
                        if (lit_pos_c != cnf_tracking.propagated_literals.end() && cb.size() + 1 < c.size()) {
                            cb.push_back(*lit_pos_c);

                            // Update mapping. c cannot be unit here.
                            std::set<int> pures;
                            for (size_t i = cb.size(); i < c.size(); i++) {
                                if(c[i] != *lit_pos_c) {
                                    removed_literals++;
                                    mapping[c[i]]--;
                                    if (mapping[c[i]] == 0 && mapping.contains(-c[i]) && mapping[-c[i]] > 0) {
                                        pures.insert(-c[i]);
                                        found_pures++;
                                    }
                                }
                            }
                            update_watchers(watchers, c, cb, step);
                            cnf[step] = std::move(cb);
                            if (!pures.empty()) {
                                step -= pureRuntimeLiteralElimination(cnf, mapping, std::move(pures),
                                                                      found_pures,
                                                                      removed_clauses, step);
                                watchers = create_watched_literal_mapping(cnf);
                            }
                            shortened = true;
                            break;
                        }
                        // otherwise we want to take the other case.
                        std::set<int> lits;
                        for (size_t lit = num_propagations + 1;
                             lit < cnf_tracking.propagated_literals.size(); lit++) {
                            if (std::find(c.begin(), c.end(), -cnf_tracking.propagated_literals[lit]) != c.end()) {
                                lits.insert(-cnf_tracking.propagated_literals[lit]);
                            }
                        }
                        if (!lits.empty()) {
                            std::vector<int> new_clause;
                            std::set<int> pures;
                            for (int lc: c) {
                                if (!lits.contains(lc)) {
                                    new_clause.push_back(lc);
                                } else {
                                    removed_literals++;
                                    mapping[lc]--;
                                    if (mapping[lc] == 0 && mapping.contains(-lc) && mapping[-lc] > 0) {
                                        pures.insert(-lc);
                                        found_pures++;
                                    }
                                }
                            }

                            update_watchers(watchers, c, cb, step);
                            cnf[step] = std::move(new_clause);

                            if (!pures.empty()) {
                                step -= pureRuntimeLiteralElimination(cnf, mapping, std::move(pures),
                                                                      found_pures,
                                                                      removed_clauses, step);
                                watchers = create_watched_literal_mapping(cnf);
                            }

                            if(cnf[step].size() == 1){
                                step -= runtimeUnitPropagation(cnf, mapping, cnf[step][0], removed_clauses,
                                                               removed_literals, step);
                                watchers = create_watched_literal_mapping(cnf);
                            }

                            shortened = true;
                            break;
                        }
                    }
                }
            }
            if (!shortened) {
                cnf_tracking.clauseIsSat[step] = false;
            } else {
                cnf_tracking.clauseIsSat[step] = false;
                change = true;
            }
            step++;
        }
    }
}


} // sorted
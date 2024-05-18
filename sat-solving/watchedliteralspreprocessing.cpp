//
// Created by Christofer Held on 25.04.24.
//

#include "watchedliteralspreprocessing.h"
#include "preprocessing.h"
#include <algorithm>

namespace watched_literals {

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
                if(std::find(unit_clauses.begin(),unit_clauses.end(), -cnf[i][0]) != unit_clauses.end()){
                    cnf = {{}};
                    return;
                }
                if(std::find(unit_clauses.begin(),unit_clauses.end(), cnf[i][0]) == unit_clauses.end()) {
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
                    if (!found_new_watcher && !sat_clauses[clause]) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other >= 0; other--) {
                            if (propagated_literals.contains(cnf[clause][other])) {
                                sat_clauses[clause] = true;
                                break;
                            }

                            if (watchers.contains(cnf[clause][other]) &&
                                std::find(watchers[cnf[clause][other]].begin(),
                                          watchers[cnf[clause][other]].end(),
                                          std::pair<int, int>{clause, other}) !=
                                watchers[cnf[clause][other]].end()) {

                                if (std::find(unit_clauses.begin(), unit_clauses.end(), -cnf[clause][other]) != unit_clauses.end()) {
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
                    if (!found_new_watcher && !runtime_info.clauseIsSat[clause]) {
                        // if we only have one watcher we are done
                        for (int other = other_watcher_pos; other >= 0; other--) {

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


    void vivify(CDNF_formula &cnf, int global_count, CSVWriter &writer) {
        // for the start we want to preprocess the clause to remove all unit clauses.
        watched_literals_unit_propagation(cnf);
        int change = 0;
        int new_change = cnf.size();

        // we store for faster Unit propagation
        auto watchers = create_watched_literal_mapping(cnf);
        int step = 0;
        auto start = high_resolution_clock::now();
        auto end = high_resolution_clock::now();
        while (new_change > 0) {

            writer.write_vivification_runtime(global_count, step, cnf.size(), (int) (change / 2), numVars(cnf),
                                              numLiterals(cnf), duration_cast<milliseconds>(end - start).count());
            new_change = 0;
            change = 0;
            step++;
            start = high_resolution_clock::now();
            for (int i = 0; i < cnf.size(); i++) {
                if (i % 1000 == 0) {
                    std::cout << "interation: " << step << " \tclause: " << i << "\t changed: " << change / 2
                              << std::endl;
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
                int pos = 0;
                while (!shortened && c != cb) {

                    int l = c[pos];//select_a_literal(c, cb);
                    pos++;
                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat = false;
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
            end = high_resolution_clock::now();
        }
    }


    std::vector<int> pureLiteralElimination(CDNF_formula& formula) {
        bool found_pures = true;
        std::vector<bool> sat_clauses = std::vector<bool>(formula.size());

        while (found_pures) {
            found_pures = false;

            std::set<int> literals = {}; // Track the polarity of each literal
            // Count the polarity of each literal in the formula
            for (size_t i = 0; i < formula.size(); ++i) {
                if (!sat_clauses[i]) {
                    for (int literal : formula[i]) {
                        literals.insert(literal);
                    }
                }
            }

            std::set<int> pures = {};
            for(auto literal: literals){
                if(literals.find(-literal) == literals.end()){
                    pures.insert(literal);
                }
            }

            for (size_t i = 0; i < formula.size(); i++) {
                if(sat_clauses[i]) { continue;}
                for (int literal: formula[i]) {
                    if (pures.find(literal) != pures.end()) {
                        found_pures= true;
                        sat_clauses[i] = true;
                    }
                }
            }
        }

        std::vector<int> clauses;
        for(int i = 0; i < formula.size() ; i++){
            if(sat_clauses[i]){
                clauses.push_back(i);
            }
        }

        std::cout << "found clauses containing Pures: " <<  clauses.size() << std::endl;

        return clauses;
    }

    void vivify_with_pure_lit(CDNF_formula &cnf, int global_count, CSVWriter &writer, CSVWriter &pure_writer) {
        // for the start we want to preprocess the clause to remove all unit clauses.
        watched_literals_unit_propagation(cnf);

        int change = 0;
        int new_change = cnf.size();

        auto start = high_resolution_clock::now();
        auto end = high_resolution_clock::now();
        // we store for faster Unit propagation
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        std::vector<int> already_sat = pureLiteralElimination(cnf);
        int step = 0;
        int sat_count = 0;
        while (new_change > 0) {
            std::vector<int> already_sat = pureLiteralElimination(cnf);
            if(already_sat.size() == cnf.size()){
                cnf = {};
                return;
            }
            sat_count += already_sat.size();
            if (!already_sat.empty()){
                CDNF_formula copy = cnf;
                cnf = {};
                int count = 0;
                for (int i = 0; i < copy.size(); i++) {
                    if (already_sat[count] < i){
                        if(already_sat.size() - 1 > count){
                            count++;
                        }
                    }
                    if(already_sat[count] == i) continue;
                    cnf.push_back(copy[i]);
                }
            }
            if(cnf.empty()) return;

            if(cnf.size() == 1 && cnf[0].empty()) return;

            already_sat = {};

            writer.write_vivification_runtime(global_count, step, cnf.size(), (int) (change / 2), numVars(cnf),
                                              numLiterals(cnf), duration_cast<milliseconds>(end - start).count());
            new_change = 0;
            change = 0;
            step++;
            start = high_resolution_clock::now();
            int pos_in_sat = 0;
            for (int i = 0; i < cnf.size(); i++) {
                if (i % 100 == 0) {
                    int bevore = already_sat.size();
                    auto start2 = high_resolution_clock::now();
                    already_sat = pureLiteralElimination(cnf);
                    auto end2 = high_resolution_clock::now();
                    if (i % 1000 == 0)
                        std::cout << "iteration: " << step << " \tclause: " << i << "\t sat_pures "
                                  << already_sat.size()
                                  << "\t changed: " << change / 2 << std::endl;
                    sat_count += already_sat.size() - bevore;
                    pure_writer.write_pure(global_count, step, i, already_sat.size() - bevore, sat_count,
                                           duration_cast<milliseconds>(end2 - start2).count());

                }

                while (already_sat[pos_in_sat] < i && pos_in_sat < already_sat.size()){
                    pos_in_sat++;
                }
                if(already_sat[pos_in_sat] == i) continue;

                // we now work with the tracking info no need to create
                runtime_info cnf_tracking = create_runtime_info(cnf, watchers);

                // we want to ignore the tracking info for now.
                cnf_tracking.clauseIsSat[i] = true;

                for (int k: already_sat) {
                    cnf_tracking.clauseIsSat[k] = true;
                }

                if (std::find(cnf_tracking.clauseIsSat.begin(), cnf_tracking.clauseIsSat.end(), false) ==
                    cnf_tracking.clauseIsSat.end()) {
                    cnf = {};
                    return;
                }

                // we take a clause
                std::vector<int> c = cnf[i];

                // we take a finished clause
                std::vector<int> cb;

                bool shortened = false;
                int pos = c.size();
                while (!shortened && c != cb) {
                    pos--;
                    int l = c[pos];//select_a_literal(c, cb);

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
                    if (cnf[i].size() == 1){
                        watched_literals_unit_propagation(cnf);
                        watchers = create_watched_literal_mapping(cnf);
                        break;
                    }
                }
            }
            new_change = change;
            end = high_resolution_clock::now();
        }
    }


    /***************************************************************************************
     * Sorted vivify with pure literal
     ***************************************************************************************/

    std::unordered_map<int, std::vector<int>> create_literal_to_clause_mapping(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<int>> mapping;
        for (int i = 0; i < cnf.size(); i++) {
            for (auto l: cnf[i]) {
                mapping[l].push_back(i);
            }
        }
        return std::move(mapping);
    }


    void sort_cnf(CDNF_formula &cnf) {
        auto mapping = create_watched_literal_mapping(cnf);
        std::unordered_map<int, int> values;

        // We know it cannot contain pures anymore
        auto compare = [&mapping](int a, int b) {
            auto min_a = std::min(mapping[a].size(), mapping[-a].size());
            auto min_b = std::min(mapping[b].size(), mapping[-b].size());
            if (min_a != min_b) {
                return min_a > min_b;
            }
            auto max_a = std::max(mapping[a].size(), mapping[-a].size());
            auto max_b = std::max(mapping[b].size(), mapping[-b].size());
            return max_a < max_b;
        };

        for (auto &clause: cnf) {
            std::sort(clause.begin(), clause.end(), compare);
        }

        auto compare2 = [&mapping](const std::vector<int> &a, const std::vector<int> &b) {
            auto get_min_size = [&mapping](int val) {
                return std::min(mapping[val].size(), mapping[-val].size());
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

        std::sort(cnf.begin(), cnf.end(), compare2);
    }



    void vivify_with_sorted_pure_lit(CDNF_formula &cnf, int global_count, CSVWriter &writer, CSVWriter &pure_writer) {
        watched_literals_unit_propagation(cnf);

        // Preprocess
        int change = 0;
        int new_change = cnf.size();

        auto start = high_resolution_clock::now();
        auto end = high_resolution_clock::now();

        // we store for faster Unit propagation
        std::unordered_map<int, std::vector<std::pair<int, int>>> watchers;
        int step = 0;
        int iterations = 500;
        int sat_count = 0;

        while (new_change > 0 || iterations < cnf.size()) {
            iterations*=2;
            auto start2 = high_resolution_clock::now();

            std::vector<int> already_sat = pureLiteralElimination(cnf);
            std::cout << already_sat.size() << std::endl;
            if(already_sat.size() == cnf.size()){
                cnf = {};
                return;
            }
            sat_count += already_sat.size();
            if (!already_sat.empty()){
                CDNF_formula copy = cnf;
                cnf = {};
                int count = 0;
                for (int i = 0; i < copy.size(); i++) {
                    if (already_sat[count] < i){
                        if(already_sat.size() - 1 > count){
                            count++;
                        }
                    }
                    if(already_sat[count] == i) continue;
                    cnf.push_back(copy[i]);
                }
            }
            if(cnf.empty()) return;

            if(cnf.size() == 1 && cnf[0].empty()) return;

            auto start_sort = high_resolution_clock::now();
            sort_cnf(cnf);
            auto end_sort = high_resolution_clock::now();
            watchers = create_watched_literal_mapping(cnf);

            auto end2 = high_resolution_clock::now();
            if (cnf.empty()) {
                return;
            }
            writer.write_vivification_runtime(global_count, step, cnf.size(), (int) (change / 2), numVars(cnf),
                                              numLiterals(cnf), duration_cast<milliseconds>(end - start).count());

            change = 0;
            step++;

            pure_writer.write_pure(global_count, step, step * 100, already_sat.size(), sat_count,
                                   duration_cast<milliseconds>(end2 - start2).count());
            if (step % 10 == 0)
                std::cout << "iteration: " << step << " \tclause: " << step * 100 << "\t sat_pures " << sat_count
                          << "\t changed: " << change / 2 << "\tpure_time: "
                          << duration_cast<milliseconds>(end2 - start2).count() << "\tvivify_time:"
                          << duration_cast<milliseconds>(end - start).count() << std::endl;

            start = high_resolution_clock::now();
            if(iterations >= cnf.size()){
                iterations = cnf.size();
            }
            for (int i = 0; i < iterations; i++) {
                if (std::find(already_sat.begin(), already_sat.end(), i) != already_sat.end()) continue;

                // we now work with the tracking info no need to create
                runtime_info cnf_tracking = create_runtime_info(cnf, watchers);

                // we want to ignore the tracking info for now.
                cnf_tracking.clauseIsSat[i] = true;
                for (int k: already_sat) {
                    cnf_tracking.clauseIsSat[k] = true;
                }

                if (std::find(cnf_tracking.clauseIsSat.begin(), cnf_tracking.clauseIsSat.end(), false) ==
                    cnf_tracking.clauseIsSat.end()) {
                    cnf = {};
                    return;
                }

                // we take a clause
                std::vector<int> c = cnf[i];

                // we take a finished clause
                std::vector<int> cb;
                bool shortened = false;
                int pos = 0;
                while (!shortened && c != cb) {
                    int l = c[pos];//select_a_literal(c, cb);
                    pos++;
                    cb.push_back(l);

                    // we work with the unit_tracking. It returns uns everything we need to know.
                    bool is_unsat = false;
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
            end = high_resolution_clock::now();
        }
    }


} // watched_literals




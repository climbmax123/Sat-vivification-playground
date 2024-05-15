//
// Created by Christofer Held on 12.04.24.
//

#include "watchedpreprocessing.h"
#include <algorithm>

namespace watched {

    void unit_propagation(CDNF_formula &cnf) {
        bool progress = true;
        while (progress) {
            progress = false;
            std::set<int> unit_clauses_set;
            std::vector<int> unit_clauses;
            for (const auto &clause: cnf) {
                if (clause.size() == 1) {
                    int unit = clause.front();
                    if (unit_clauses_set.find(-unit) != unit_clauses_set.end()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
                    }
                    if (unit_clauses_set.insert(unit).second) {
                        unit_clauses.push_back(unit);
                    }
                }
            }
            if (unit_clauses.empty()) {
                break;
            }
            for (int unit: unit_clauses) {
                cnf.erase(std::remove_if(cnf.begin(), cnf.end(),
                                         [unit](const std::vector<int> &clause) {
                                             return std::find(clause.begin(), clause.end(), unit) != clause.end();
                                         }), cnf.end());

                for (auto &clause: cnf) {
                    clause.erase(std::remove(clause.begin(), clause.end(), -unit), clause.end());
                    if (clause.empty()) {
                        cnf = CDNF_formula(1);
                        cnf.emplace_back();
                        return;
                    }
                }
                progress = true;
            }
        }
    }


    std::unordered_map<int, std::vector<int>> create_literal_to_clause_mapping(CDNF_formula &cnf) {
        std::unordered_map<int, std::vector<int>> mapping;
        for (int i = 0; i < cnf.size(); i++) {
            for (auto l: cnf[i]) {
                mapping[l].push_back(i);
            }
        }
        return std::move(mapping);
    }

    struct runtime_info {
        std::unordered_map<int, size_t> instances;
        std::vector<int> propagated_literals;
        std::vector<bool> clauseIsSat;
    };

    runtime_info create_runtime_info(CDNF_formula &cnf){
        runtime_info runtimeInfo;
        runtimeInfo.clauseIsSat = std::vector<bool>(cnf.size(), false);
        for(int i = 0; i < cnf.size(); i++){
            runtimeInfo.instances[i] = cnf[i].size();
        }
        runtimeInfo.propagated_literals = std::vector<int>();
        return std::move(runtimeInfo);
    }

    int select_a_literal(const std::vector<int> &c, const std::vector<int> &cb) {
        for (int lit: c) {
            if (std::find(cb.begin(), cb.end(), lit) == cb.end()) {
                return lit;
            }
        }
        return 0;
    }


    void UP(std::unordered_map<int, std::vector<int>> &cnf_mapping,
            runtime_info &unit_tracking,
            CDNF_formula &cnf,
            int new_unit,
            bool &is_unsat
            ){
        std::vector<int> found_units;
        found_units.push_back(new_unit);
        while (!found_units.empty()) {
            int current_unit = found_units[0];
            found_units.erase(found_units.begin());
            unit_tracking.propagated_literals.push_back(current_unit);
            for (int clause: cnf_mapping[current_unit]) {
                unit_tracking.clauseIsSat[clause] = true;
            }

            for (int clause: cnf_mapping[-current_unit]) {
                if(unit_tracking.clauseIsSat[clause]) continue;
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


    void update_mapping(std::unordered_map<int, std::vector<int>> &mapping, std::vector<int> &c, std::vector<int> &cb, int pos){
        for (int lit: c){
            bool find_pos = std::find(cb.begin(),cb.end(),lit) != cb.end() ;
            bool find_neg = std::find(cb.begin(),cb.end(),-lit) != cb.end();
            if (!find_neg && !find_pos){
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

                    UP(cnf_mapping, cnf_tracking, cnf, -l,  is_unsat);

                    if(std::find(cnf_tracking.clauseIsSat.begin(),cnf_tracking.clauseIsSat.end(), false) == cnf_tracking.clauseIsSat.end()){
                        cnf = CDNF_formula();
                        return;
                    }

                    if (is_unsat) {
                        update_mapping(cnf_mapping,c,cb,i);
                        cnf[i] = cb;
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for(size_t lit = num_propagations + 1; lit < cnf_tracking.propagated_literals.size(); lit++){
                            int unit = cnf_tracking.propagated_literals[lit];

                            if(std::find(c.begin(),c.end(), unit) != c.end()){
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    update_mapping(cnf_mapping,c,cb,i);
                                    cnf[i] = cb;
                                    shortened = true;
                                }
                                break;
                            }

                            if(std::find(c.begin(),c.end(), -unit) != c.end()){
                                std::vector<int> new_clause;
                                for (int lc: c) {
                                    if (lc != -unit) {
                                        new_clause.push_back(lc);
                                    }
                                }
                                update_mapping(cnf_mapping,c,cb,i);
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

    void find_pures(std::unordered_map<int, std::vector<int>> &cnf_mapping, std::vector<bool> &clauses){
        // Iterate over the map
        for (const auto& pair : cnf_mapping) {
            int key = pair.first;
            int neg_key = -key;

            // Check if the negated key does not exist in the map
            if (cnf_mapping.find(neg_key) == cnf_mapping.end()) {
                for (const auto& clause: pair.second){
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

            if(std::find(contains_pures.begin(), contains_pures.end(), false) == contains_pures.end()){
                cnf = CDNF_formula();
                return;
            }

            for (int i = 0; i < cnf.size(); i++) {
                if(contains_pures[i]){
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

                    UP(cnf_mapping, cnf_tracking, cnf, -l,  is_unsat);

                    if(std::find(cnf_tracking.clauseIsSat.begin(),cnf_tracking.clauseIsSat.end(), false) == cnf_tracking.clauseIsSat.end()){
                        cnf = CDNF_formula();
                        return;
                    }

                    if (is_unsat) {
                        update_mapping(cnf_mapping,c,cb,i);
                        cnf[i] = cb;
                        if (c != cb) {
                            shortened = true;
                            change = true;
                        }
                    } else {
                        for(size_t lit = num_propagations + 1; lit < cnf_tracking.propagated_literals.size(); lit++){
                            int unit = cnf_tracking.propagated_literals[lit];

                            if(std::find(c.begin(),c.end(), unit) != c.end()){
                                if (cb.size() + 1 < c.size()) {
                                    cb.push_back(unit);
                                    update_mapping(cnf_mapping,c,cb,i);
                                    cnf[i] = cb;
                                    shortened = true;
                                }
                                break;
                            }

                            if(std::find(c.begin(),c.end(), -unit) != c.end()){
                                std::vector<int> new_clause;
                                for (int lc: c) {
                                    if (lc != -unit) {
                                        new_clause.push_back(lc);
                                    }
                                }
                                update_mapping(cnf_mapping,c,cb,i);
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

        for(size_t i=contains_pures.size()-1; i >= 0; i--){
            if(contains_pures[i]) {
                cnf.erase(cnf.begin() + i);
            }
        }
    }
}
//
// Created by Christofer Held on 02.06.24.
//

#include "combinedpreprocessing.h"

namespace combined {

    std::unordered_map<int, int> creat_mapping(CDNF_formula &formula) {
        std::unordered_map<int, int> map;
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

    void pureLiteralElimination(CDNF_formula &formula, std::unordered_map<int, int> &mapping) {
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


    void unit_propagation(CDNF_formula &cnf, std::unordered_map<int, int> &mapping) {
        std::vector<int> unit_clauses;

        for(auto lit: mapping){
            if(lit.second == 1){
                unit_clauses.push_back(lit.first);
            }
        }

        std::set<int> propagated_literals;

        int sum_val = 0;
        int count = 0;
        while (count < unit_clauses.size()) {
            int lit = unit_clauses[count];
            propagated_literals.insert(lit);
            if(propagated_literals.contains(-lit)){
                cnf = {{}};
                return;
            }
            CDNF_formula new_cnf;
            int s_count = 0;
            for (auto &clause: cnf) {
                if (std::find(clause.begin(), clause.end(), lit) != clause.end()) {
                    for (auto i: clause) {
                        mapping[i]--;
                    }
                } else if (std::find(clause.begin(), clause.end(), -lit) != clause.end()) {
                    clause.erase(std::find(clause.begin(), clause.end(), -lit));
                    mapping[-lit]--;

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
    }


    void combined_methode(CDNF_formula& cnf, int timeLimitInSeconds){
        auto startTime = std::chrono::steady_clock::now();
        auto mapping = creat_mapping(cnf);
        int size = 0;
        while (cnf.size() != size) {
            size = cnf.size();
            if (timeLimitInSeconds != -1) {
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                        currentTime - startTime).count();
                if (elapsedSeconds >= timeLimitInSeconds) {
                    std::cout << "Timeout reached ending vivify" << std::endl;
                    return;
                }
            }
            pureLiteralElimination(cnf, mapping);
            unit_propagation(cnf, mapping);
        }
    }

}
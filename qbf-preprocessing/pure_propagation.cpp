//
// Created by Christofer Held on 27.06.24.
//

#include "pure_propagation.h"
#include <map>
#include <set>
#include <iostream>

namespace pure {

    std::map<int, std::pair<int, bool>> creat_mapping(QBF qbf) {
        std::map<int, std::pair<int, bool>> map;
        for (const auto &cl: qbf.formula) {
            for (int i: cl) {
                if (map.contains(i)) {
                    map[i].first++;
                } else {
                    map.insert({i, {1, qbf.quantifierTypeIsExists[std::abs(i)]}});
                }
            }
        }
        return std::move(map);
    }

    void pure_propagation_with_universals(QBF &qbf) {
        auto mapping = creat_mapping(qbf);

        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        std::set<std::pair<int, bool>> pures = {};
        for (auto literal: mapping) {
            if (mapping.contains(-literal.first) && mapping[literal.first].first > 0) {
                continue;
            }
            pures.insert({literal.first, mapping[literal.first].second});
        }

        while (!pures.empty()) {
            std::set<std::pair<int, bool>> new_pures = {};
            for (size_t i = 0; i < qbf.formula.size(); i++) {
                if (sat_clauses[i]) { continue; }
                for (int j = 0; j < qbf.formula[i].size(); j++){
                    if (pures.find({qbf.formula[i][j], qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][j])]}) != pures.end()) {
                        if (qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][j])]) {
                            sat_clauses[i] = true;
                            for (int l: qbf.formula[i]) {
                                mapping[l].first--;
                                if (mapping[l].first == 0 && mapping[-l].first > 0) {
                                    new_pures.insert({-l, qbf.quantifierTypeIsExists[std::abs(l)]});
                                }
                            }
                            break;
                        } else {
                            qbf.formula[i].erase(qbf.formula[i].begin()+j);
                            j--;
                        }
                    }
                }
            }
            pures = new_pures;
        }

        if(std::find(sat_clauses.begin(), sat_clauses.end(), false) == sat_clauses.end()){
            qbf.quantifierTypeIsExists[1] = true;
            qbf.formula = {{1}};
            qbf.quantifierOrder = {1};
            return;
        }
        std::vector<std::vector<int>> newFormula;
        for (int i = 0; i < qbf.formula.size(); i++) {
            if (!sat_clauses[i]) {
                newFormula.push_back(std::move(qbf.formula[i]));
            }
        }

        std::vector<int> qu_ord;
        for(auto q: qbf.quantifierOrder){
            bool found = false;
            for(auto c: newFormula){
                for(auto l : c){
                    if(std::abs(l) == q){
                        found = true;
                        break;
                    }
                }
                if(found){
                    qu_ord.push_back(q);
                    break;
                }
            }
        }
        qbf.formula = std::move(newFormula);
        qbf.quantifierOrder = std::move(qu_ord);
    }

    void pure_propagation_without_universals(QBF &qbf) {
        auto mapping = creat_mapping(qbf);

        std::vector<bool> sat_clauses = std::vector<bool>(qbf.formula.size());

        std::set<std::pair<int, bool>> pures = {};
        for (auto literal: mapping) {
            if (mapping.contains(-literal.first) && mapping[literal.first].first > 0) {
                continue;
            }
            if(mapping[literal.first].second) {
                pures.insert({literal.first, mapping[literal.first].second});
            }
        }

        int step = 0;
        while (!pures.empty()) {
            std::cout << "Step " << step++ << ": \t";
            for(auto p : pures){
                std::cout << " " << std::abs(p.first);
            }
            std::cout << std::endl;
            std::set<std::pair<int, bool>> new_pures = {};
            for (size_t i = 0; i < qbf.formula.size(); i++) {
                if (sat_clauses[i]) { continue; }
                for (int j = 0; j < qbf.formula[i].size(); j++){
                    if (pures.find({qbf.formula[i][j], qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][j])]}) != pures.end()) {
                        if (qbf.quantifierTypeIsExists[std::abs(qbf.formula[i][j])]) {
                            sat_clauses[i] = true;
                            for (int l: qbf.formula[i]) {
                                mapping[l].first--;
                                if (mapping[l].first == 0 && mapping[-l].first > 0) {
                                    if(qbf.quantifierTypeIsExists[std::abs(l)]) {
                                        new_pures.insert({-l, qbf.quantifierTypeIsExists[std::abs(l)]});
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
            pures = new_pures;
        }

        if(std::find(sat_clauses.begin(), sat_clauses.end(), false) == sat_clauses.end()){
            qbf.quantifierTypeIsExists[1] = true;
            qbf.formula = {{1}};
            qbf.quantifierOrder = {1};
            return;
        }
        std::vector<std::vector<int>> newFormula;
        for (int i = 0; i < qbf.formula.size(); i++) {
            if (!sat_clauses[i]) {
                newFormula.push_back(std::move(qbf.formula[i]));
            }
        }

        std::vector<int> qu_ord;
        for(auto q: qbf.quantifierOrder){
            bool found = false;
            for(auto c: newFormula){
                for(auto l : c){
                    if(std::abs(l) == q){
                        found = true;
                        break;
                    }
                }
                if(found){
                    qu_ord.push_back(q);
                    break;
                }
            }
        }
        qbf.formula = std::move(newFormula);
        qbf.quantifierOrder = std::move(qu_ord);
    }
} // pure
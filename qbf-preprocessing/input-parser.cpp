//
// Created by christofer on 26.04.24.
//
#include <set>
#include "input-parser.h"


QBF parse_DIMACS_main(std::istream &in) {
    QBF qbf;
    std::string line;

    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::vector<std::string> words;
        std::string word;
        while (std::getline(iss, word, ' ')) {
            words.push_back(word);
        }
        if (words[0] == "c") {
            continue;
        }
        if (words[0] == "p" && words[1] == "cnf") {
            std::cout << "found variables: " << std::stoi(words[2]) << " and clauses " << std::stoi(words[3])
                      << std::endl;
            continue;
        }

        if (words[0] == "a") {
            int i = 1;
            while (words[i] != "0") {
                qbf.quantifiers.emplace_back(QuantifierType::FORALL, std::stoi(words[i]));
                i++;
            }
            continue;
        }

        if (words[0] == "e") {
            int i = 1;
            while (words[i] != "0") {
                qbf.quantifiers.emplace_back(QuantifierType::EXISTS, std::stoi(words[i]));
                i++;
            }
            continue;
        }

        std::vector<int> vc = std::vector<int>(words.size() - 1);
        for (int j = 0; j < words.size() - 1; j++) {
            vc[j] = std::stoi(words[j]);
        }

        std::vector<int> sorted_vc;
        for (const auto &qt: qbf.quantifiers) {
            if(std::find(vc.begin(), vc.end(), qt.second) != vc.end()){
                sorted_vc.emplace_back(qt.second);
            }
            if(std::find(vc.begin(), vc.end(), -qt.second) != vc.end()){
                sorted_vc.emplace_back(-qt.second);
            }
        }

        qbf.formula.push_back(std::move(sorted_vc));
    }
    return qbf;
}


// Function to parse QDIMACS from a file
QBF parseQDIMACSFromFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file: " + filename);
    }
    return parse_DIMACS_main(file);
}


void writeQDIMACS(const QBF& qbf, const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::set<int> vars;
    for(auto const cl : qbf.formula){
        for(auto const lit: cl){
            vars.insert(std::abs(lit));
        }
    }
    outfile << "p cnf " << vars.size() << " " << qbf.formula.size() << std::endl;

    // Write quantifiers
    for (const auto& [qt, var] : qbf.quantifiers) {
        if (qt == EXISTS) {
            outfile << "e " << var  << " 0"<< std::endl;
        } else if (qt == FORALL) {
            outfile << "a " << var << " 0" << std::endl;
        } else {
            // Handle undefined quantifier (optional: warn or throw exception)
            std::cerr << "Warning: Unknown quantifier type in QBF struct" << std::endl;
        }
    }

    // Write clauses
    for (const auto& clause : qbf.formula) {
        outfile << " ";
        for (const auto& lit : clause) {
            outfile << lit << " ";
        }
        outfile << "0" << std::endl; // Mark the end of the clause
    }

    outfile.close();
}



void printQBF(const QBF &qbf) {
    for (const auto &qt: qbf.quantifiers) {
        if (qt.first == EXISTS) {
            std::cout << "E ";
        } else if (qt.first == FORALL) {
            std::cout << "A ";
        }
        std::cout << qt.second << " ";
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


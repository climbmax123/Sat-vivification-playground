//
// Created by christofer on 26.04.24.
//
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


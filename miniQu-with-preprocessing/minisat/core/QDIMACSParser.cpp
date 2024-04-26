//
// Created by Christofer Held on 23.04.24.
//

#include "QDIMACSParser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <assert.h>
#include <algorithm>

using std::make_tuple;

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); }
    );
    return s;
}

QDIMACSParser::QDIMACSParser() {};

QDIMACSParser::QDIMACSParser(const string &filename)  {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        line = str_tolower(line);
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.empty() || line.front() == 'c') {
            continue; // Skip comments and empty lines
        } else if (line.front() == 'p') {
            // Process preamble here if necessary
            continue;
        } else if (line.front() == 'a' || line.front() == 'e') {
            std::stringstream ss(line);
            char quantifier;
            int var;
            vector<int> vars;
            ss >> quantifier; // First token is the quantifier 'a' or 'e'
            while (ss >> var && var != 0) {
                vars.push_back(var);
                string var_id = std::to_string(var);
                VariableType type = (quantifier == 'e') ? Existential : Universal;
                addVariable(var_id, type);
            }
            quantifier_blocks.push_back({vars});
            max_quantifier_depth = std::max(max_quantifier_depth, static_cast<unsigned int>(quantifier_blocks.size()));
        } else {
            // Assume the line is a clause in the CNF
            std::stringstream ss(line);
            int lit;
            vector<int> clause;
            while (ss >> lit && lit != 0) {
                clause.push_back(lit);
            }
            gates.push_back(clause); // Assuming a simpler representation for gates
        }
    }
    std::cout << "Parsed " << gates.size() << " clauses and " << quantifier_blocks.size() << " quantifier blocks." << std::endl;

}

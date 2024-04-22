//
// Created by Christofer Held on 11.04.24.
//

#include "QBFTester.h"

int QBFTester::countVariables(const CDNF_formula& clauses) {
    std::set<int> vars;
    for (const auto& clause : clauses) {
        for (int var : clause) {
            vars.insert(std::abs(var));
        }
    }
    return vars.size();
}

void QBFTester::saveQBFToFile(const std::string &filePath, const CDNF_formula &clauses, const CDNF_formula &terms) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Write comments and problem line
    file << "c Saved QBF formula in QDIMACS format\n";
    file << "p cnf " <<     countVariables(clauses) << " " << clauses.size() << "\n";

    // Write quantifiers
    std::map<int, char> quantMap; // to store quantifier types
    for (int var : quantifiers) {
        char quantType = (var > 0) ? 'e' : 'a'; // Assuming positive for existential, negative for universal
        int absVar = std::abs(var);
        quantMap[absVar] = quantType;
    }

    // Print quantifiers by their type
    if (!quantMap.empty()) {
        char currentQuant = '\0';
        for (const auto& entry : quantMap) {
            if (entry.second != currentQuant) {
                if (currentQuant != '\0') file << "0\n"; // End previous quantifier line
                currentQuant = entry.second;
                file << currentQuant << " ";
            }
            file << entry.first << " ";
        }
        file << "0\n"; // End last quantifier line
    }

    // Write clauses
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            file << lit << " ";
        }
        file << "0\n";
    }

    file.close();
    std::cout << "QBF formula saved to " << filePath << std::endl;
}

bool QBFTester::callQbfSolver(const std::string &qbfFilePath, const std::string &resultFilePath) {
    return false;
}

QBFTester::QBFTester(int variables, int clauses) : num_variables(variables), num_clauses(clauses) {
    srand(static_cast<unsigned int>(time(nullptr)));
}

std::pair<CDNF_formula, CDNF_formula> QBFTester::generateQBF() {
    std::string output = exec("python qbfuzz.py"); // Ensure your path and environment are correctly set
    return parseQDIMACS(output);
}

std::string QBFTester::exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::pair<CDNF_formula, CDNF_formula> QBFTester::parseQDIMACS(const std::string& qdimacs) {
    std::istringstream iss(qdimacs);
    std::string line;
    CDNF_formula clauses;
    CDNF_formula terms; // Use this to store terms, assuming they're marked in the QDIMACS input
    bool readingTerms = false; // Flag to switch from reading clauses to terms

    while (getline(iss, line)) {
        std::istringstream lineStream(line);
        char type;
        lineStream >> type;
        if (type == 'p') {
            // Handle problem line
            continue;
        } else if (type == 'c') {
            // Check if the comment indicates the start of terms
            std::string comment;
            getline(lineStream, comment); // Get the rest of the line
            if (comment.find("terms start") != std::string::npos) {
                readingTerms = true; // Start reading terms from now on
            }
            continue;
        } else if (type == 'e' || type == 'a') {
            // Quantifiers are handled here, potentially resetting term reading
            readingTerms = false;
        } else {
            // Must be a clause line
            std::vector<int> clause;
            int literal;
            while (lineStream >> literal && literal != 0) {
                clause.push_back(literal);
            }
            if (!clause.empty()) {
                if (readingTerms) {
                    terms.push_back(clause);
                } else {
                    clauses.push_back(clause);
                }
            }
        }
    }

    return {clauses, terms};
}

bool QBFTester::testQBF(const CDNF_formula &adapted_clauses, const CDNF_formula &adapted_terms) {
    return false;
}

void QBFTester::printOriginalQBF() {

}

void QBFTester::printQBF(const CDNF_formula &clauses, const CDNF_formula &terms) {

}

void QBFTester::measureDifference(const CDNF_formula &clauses, const CDNF_formula &terms) {

}

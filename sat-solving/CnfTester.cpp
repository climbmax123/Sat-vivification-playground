#include "CnfTester.h"


CNFTester::CNFTester(int variables, int clauses) : num_variables(variables), num_clauses(clauses) {
    srand(time(0));
    this->current_debug_clause = 0;// Initialisierung des Zufallsgenerators
}

std::string CNFTester::generateLiteral(int variable) {
    std::string literal = (rand() % 2 == 0) ? "-" : "";
    literal += std::to_string(variable);
    return literal;
}

bool callMinisat(const std::string &cnfFilePath, const std::string &resultFilePath) {
    std::string command = "minisat " + cnfFilePath + " " + resultFilePath;
    int result = system(command.c_str());

    if (result == 0) {
        std::ifstream resultFile(resultFilePath);
        std::string line;
        if (resultFile.is_open()) {
            while (getline(resultFile, line)) {
                if (line == "SAT") {
                    return true; // CNF ist erfüllbar
                } else if (line == "UNSAT") {
                    return false; // CNF ist unerfüllbar
                }
            }
            resultFile.close();
        } else {
            std::cerr << "Unable to open result file." << std::endl;
        }
    } else {
        std::cerr << "Minisat execution failed." << std::endl;
    }

    return false; // Standardmäßig als unerfüllbar behandeln, wenn etwas schiefgeht
}


CDNF_formula CNFTester::generateCNF() {
    CDNF_formula cnf_formula;
    for (int i = 0; i < num_clauses; ++i) {
        std::vector<int> clause;
        int num_literals = rand() % num_clauses + 1;

        std::set<int> unique_literals = {};
        for (int i = 0; i < num_literals; i++) {
            int variable = (rand() % num_variables + 1) * (rand() % 2 == 0 ? 1 : -1);
            if (!unique_literals.contains(abs(variable))) {
                unique_literals.insert(abs(variable));
                clause.push_back(variable);
            }
        }

        cnf_formula.push_back(clause);
    }
    original_cnf = cnf_formula;
    return cnf_formula;
}


void CNFTester::saveCNFToFile(const std::string &filePath, const CDNF_formula &cnf) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Unable to open file for writing: " << filePath << std::endl;
        return;
    }

    // DIMACS-Header schreiben
    int numClauses = cnf.size();
    int numVariables = 0; // Diese könnte man berechnen, falls notwendig
    file << "p cnf " << numVariables << " " << numClauses << "\n";

    // CNF-Formel schreiben
    for (const auto &clause: cnf) {
        for (int literal: clause) {
            file << literal << " ";
        }
        file << "0\n"; // Ende einer Klausel markieren
    }
}

bool CNFTester::callMinisat(const CDNF_formula &cnf) {
    const std::string cnfFilePath = "input.cnf";
    const std::string resultFilePath = "result.txt";

    // Save CNF formula to file
    saveCNFToFile(cnfFilePath, cnf);

    // Construct command and call MiniSat
    std::string command = "minisat " + cnfFilePath + " " + resultFilePath + " > minisat_output.txt 2>&1";
    int systemResult = system(command.c_str());

    // Check MiniSat execution result
    if (WIFEXITED(systemResult)) {
        int exitStatus = WEXITSTATUS(systemResult);
        switch (exitStatus) {
            case 10: // SAT
                return true;
            case 20: // UNSAT
                return false;
            default:
                std::cerr << "Minisat execution failed with unexpected exit code: " << exitStatus << std::endl;
                return false;
        }
    } else {
        std::cerr << "Minisat did not terminate normally." << std::endl;
        return false;
    }
}


bool CNFTester::testCNF(const CDNF_formula &adapted_cnf) {
    return callMinisat(original_cnf) == callMinisat(adapted_cnf);
}

void CNFTester::printOriginalCNF() {
    std::cout << "Original CNF:" << std::endl;
    printCNF(this->original_cnf);
}

void CNFTester::printCNF(const CDNF_formula &cnf) {
    for (const auto &clause: cnf) {
        std::cout << "(";
        for (const auto &literal: clause) {
            std::cout << literal << " ";
        }
        std::cout << ") ";
    }
    std::cout << std::endl;
}


void CNFTester::measureDifference(const CDNF_formula &cnf) {
    int original_clauses = original_cnf.size();
    int given_clauses = cnf.size();
    int clause_diff = std::abs(original_clauses - given_clauses);

    int original_literals = 0;
    for (const auto &clause: original_cnf) {
        original_literals += clause.size();
    }

    int given_literals = 0;
    for (const auto &clause: cnf) {
        given_literals += clause.size();
    }

    int literal_diff = std::abs(original_literals - given_literals);

    std::cout << "Difference in clauses: " << clause_diff << std::endl;
    std::cout << "Difference in literals: " << literal_diff << std::endl;
}

void CNFTester::saveCNF(const std::string &path) {
    std::string filename = generateUUID(path); // Verwende den angepassten Pfad
    std::ofstream file(path + "/" + filename);
    if (file.is_open()) {
        for (const auto &clause: original_cnf) {
            for (const auto &literal: clause) {
                file << literal << " ";
            }
            file << "0\n"; // CNF-Ende einer Klausel
        }
        file.close();
        std::cout << "CNF saved to: " << path + "/" + filename << std::endl;
    } else {
        std::cerr << "Unable to open file for writing: " << filename << std::endl;
    }
}


CDNF_formula CNFTester::loadCNF(const std::string &path) {
    original_cnf.clear();
    std::ifstream file(path);
    if (file.is_open()) {
        std::string line;
        while (getline(file, line)) {
            std::istringstream iss(line); // Jetzt funktioniert dies, da <sstream> inkludiert ist
            std::vector<int> clause;
            int literal;
            while (iss >> literal) {
                if (literal == 0) break; // End of clause
                clause.push_back(literal);
            }
            if (!clause.empty()) {
                original_cnf.push_back(clause);
            }
        }
        file.close();
        std::cout << "CNF loaded from: " << path << std::endl;
    } else {
        std::cerr << "Unable to open file for reading: " << path << std::endl;
    }
    return original_cnf;
}

std::string CNFTester::generateUUID(const std::string &input_path) {
    int uuid = 1;
    std::string filename;
    do {
        filename = input_path + "/CNF_" + std::to_string(uuid) + ".cnf";
        std::ifstream file(filename);
        if (!file.good()) {
            break;
        }
        uuid++;
    } while (true);

    return "CNF_" + std::to_string(uuid) + ".cnf";
}

CDNF_formula CNFTester::deltaDebug() {
    if(current_debug_clause >= original_cnf.size()){
        current_debug_clause = 0;
    }
    new_reduces_cnf = original_cnf;
    new_reduces_cnf.erase(new_reduces_cnf.begin()+current_debug_clause);
    return new_reduces_cnf;
}

void CNFTester::applyReduce(bool apply) {
    if(apply){
        original_cnf = new_reduces_cnf;
    } else {
        current_debug_clause++;
    }
}


CDNF_formula CNFTester::deltaLiteralDebug(){
    int lit = 0;
    for(auto cl: original_cnf){
        for(int i : cl){
            if(!this->reduced.contains(std::abs(i))){
                lit = i;
                reduced.insert(i);
            }
        }
    }
    this->new_reduces_cnf = original_cnf;
    for (auto& vec : new_reduces_cnf) {
       std::remove(vec.begin(), vec.end(), lit);
       std::remove(vec.begin(), vec.end(), -lit);
    }
    return new_reduces_cnf;
}

void CNFTester::applyLiteralReduce(bool apply){
    if(apply) {
        original_cnf = new_reduces_cnf;
    }
}



int CNFTester::size(){
    return original_cnf.size();
}

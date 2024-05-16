#include <iostream>
#include "CnfTester.h"
#include "../Preprocessor.h"
#include "preprocessing.h"
#include "fastpreprocessing.h"
#include "watchedpreprocessing.h"
#include "watchedliteralspreprocessing.h"
#include "CSVWriter.h"
#include <chrono>

int numLiterals(const CDNF_formula &cnf) {
    std::set<int> literals;
    for (const auto &clause: cnf) {
        for (const auto &lit: clause) {
            literals.insert(std::abs(lit));
        }
    }
    return literals.size();
}

bool equals(CDNF_formula &s1, CDNF_formula &s2) {
    if (s1.size() != s2.size()) return false;
    for (int i = 0; i < s1.size(); i++) {
        if (s1[i].size() != s2[i].size()) return false;
        for (int j = 0; j < s1[i].size(); j++) {
            if (s1[i][j] != s2[i][j]) return false;
        }
    }
    return true;
}

#include <omp.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;


#include "CSVWriter.h"

std::vector<std::string> findCnfFiles(const std::string &directoryPath) {
    std::vector<std::string> cnfFiles;

    try {
        // Iteriere durch das gegebene Verzeichnis und prüfe jede Datei
        for (const auto &entry: fs::recursive_directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                // Überprüfe, ob die Dateiendung .cnf ist
                if (entry.path().extension() == ".cnf") {
                    // Füge den absoluten Pfad zur Liste hinzu
                    cnfFiles.push_back(entry.path().string());
                }
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Fehler beim Durchsuchen des Verzeichnisses: " << e.what() << '\n';
    }

    return cnfFiles;
}

/*
int main() {
    CSVWriter writer("runtime_statistics7.csv");

    CNFTester tester[] = {
            CNFTester(5, 10),
            CNFTester(1 * 10, 10),
            CNFTester(2 * 10, 10),
            CNFTester(3 * 10, 10),
            CNFTester(4 * 10, 10),
            CNFTester(2 * 100, 100),
            CNFTester(4 * 100, 100),
            CNFTester(6 * 100, 100),
            CNFTester(8 * 100, 100),
            CNFTester(10 * 100, 100),
            CNFTester(12 * 1000, 1000),
            CNFTester(16 * 1000, 1000),
            CNFTester(20 * 1000, 1000),
            CNFTester(24 * 1000, 1000),
            CNFTester(28 * 1000, 1000),
    };


#pragma omp parallel for
    for (int vars = 0; vars < 15; vars += 1) {
#pragma omp parallel for
        for (int test = 0; test < 100; test++) {
            CDNF_formula cnf;
#pragma omp critical
            {
                cnf = tester[vars].generateCNF();
            }
            // normal unit prop
            CDNF_formula unit_prop = normal::unit_propagation(cnf);
            // normal pure literal elimination
            CDNF_formula pure_literal = normal::pureLiteralElimination(cnf);
            //std::cout << "vars: \t " << vars << "\tclauses: \t " << clauses << "\t test: \t " << test << std::endl;
            // combination
            CDNF_formula comb = normal::pureLiteralElimination(cnf);
            CDNF_formula new_comb = normal::unit_propagation(comb);
            while (!equals(comb, new_comb) && !cnf.empty() && !cnf[0].empty()) {
                comb = new_comb;
                new_comb = normal::unit_propagation(normal::pureLiteralElimination(comb));
            }

#pragma omp critical
            writer.writeData(cnf.size(), numLiterals(cnf), unit_prop.size(), numLiterals(unit_prop),
                             pure_literal.size(),
                             numLiterals(pure_literal), new_comb.size(), numLiterals(new_comb));
        }
    }
    return 0;
}
 */
using namespace std::chrono;

int main() {
    CSVWriter writer("runtime_statistics_cnf-val-2022.csv",
                     "num_clauses,num_literals,red_unit_clauses,red_unit_literals,unit_duration,red_pure_clauses,red_pure_literals,pure_duration,red_vivify_clauses,red_vivify_literals,vivify_duration,red_comb_up_clauses,red_comb_up_literals,up_duration,red_comb_vp_clauses,red_comb_vp_literals,vp_duration");
    CSVWriter runtime_writer("runtime_vivification_progress-2022.csv",
                             "count,step,num_clauses,schanged_clauses,total_num_vars,num_literals,duration");
    CSVWriter runtime_writer2("runtime_vivification_progress-with-pure-2022.csv",
                              "count,step,num_clauses,schanged_clauses,total_num_vars,num_literals,duration");

    CSVWriter pure_lit_writer2("runtime_pure_progress-2022.csv",
                              "count,step,iteration,found_pures,reduced_clauses_count,duration");

    CSVWriter runtime_writer3("runtime_vivification_progress-with-sorted-pure-2022.csv",
                              "count,step,num_clauses,schanged_clauses,total_num_vars,num_literals,duration");

    CSVWriter pure_lit_writer3("runtime_sorted_pure_progress-2022.csv",
                              "count,step,iteration,found_pures,reduced_clauses_count,duration");

    CNFTester tester(20000, 20000);
    auto paths = findCnfFiles("/home/christofer/Dokumente/bachelor/cnf-val-2022/");
    std::cout << paths.size() << std::endl;
    int i = 0;
//#pragma omp parallel for num_threads(16)
    for (auto const &path: paths) {
        CDNF_formula cnf;
        int rank;
//#pragma omp critical
        {

            cnf = tester.generateCNF();

            rank = i++;

            std::cout << "Loaded " << i << "\t\t   clauses:\t " << cnf.size() << "\t literals: \t" << numLiterals(cnf)
                      << std::endl;
        }
        // normal unit prop
        /*
        auto start1 = high_resolution_clock::now();
        CDNF_formula unit_prop;
        watched_literals::watched_literals_unit_propagation(cnf);


        auto end1 = high_resolution_clock::now();
        // normal pure literal elimination
        auto start2 = high_resolution_clock::now();
        CDNF_formula pure_literal = normal::pureLiteralElimination(cnf);
        auto end2 = high_resolution_clock::now();
         */
        // vivify

        CDNF_formula vivify = cnf;
        auto start3 = high_resolution_clock::now();
        //watched_literals::vivify(vivify, rank, runtime_writer);
        auto end3 = high_resolution_clock::now();
        auto duration3 = duration_cast<milliseconds>(end3 - start3).count();


        CDNF_formula vivify2 = cnf;
        auto start4 = high_resolution_clock::now();
        //watched_literals::vivify_with_pure_lit(vivify2, rank, runtime_writer2, pure_lit_writer2);
        auto end4 = high_resolution_clock::now();
        auto duration4 = duration_cast<milliseconds>(end4 - start4).count();

        CDNF_formula vivify3 = cnf;
        auto start5 = high_resolution_clock::now();
        watched_literals::vivify_with_sorted_pure_lit(vivify3, rank, runtime_writer3, pure_lit_writer3);
        auto end5 = high_resolution_clock::now();
        auto duration5 = duration_cast<milliseconds>(end5 - start5).count();



        std::cout << "Vivification Times (in milliseconds):" << std::endl;
        std::cout << "Without pure literals: " << duration3 << " ms" << std::endl;
        std::cout << "With pure literals: " << duration4 << " ms" << std::endl;
        std::cout << "With sorted pure literals: " << duration5 << " ms" << std::endl;


        /*
        // combination unit propagation pure
        auto start4 = high_resolution_clock::now();
        CDNF_formula comb = normal::pureLiteralElimination(cnf);

        CDNF_formula new_comb = comb;
        watched_literals::watched_literals_unit_propagation(new_comb);

        while (!equals(comb, new_comb) && !cnf.empty() && !cnf[0].empty()) {
            comb = new_comb;
            new_comb =normal::pureLiteralElimination(comb);
            watched_literals::watched_literals_unit_propagation(new_comb);
        }
        auto end4 = high_resolution_clock::now();
        auto start5 = high_resolution_clock::now();
        CDNF_formula comb1 = normal::pureLiteralElimination(cnf);
        CDNF_formula new_comb1 = normal::unit_propagation(comb);

        while (!equals(comb1, new_comb1) && !cnf.empty() && !cnf[0].empty()) {
            comb1 = new_comb1;
            new_comb1 = normal::pureLiteralElimination(comb1);
            watched_literals::vivify(new_comb1);
        }
        auto end5 = high_resolution_clock::now();

        auto duration1 = duration_cast<milliseconds>(end1 - start1).count();
        auto duration2 = duration_cast<milliseconds>(end2 - start2).count();
        auto duration3 = duration_cast<milliseconds>(end3 - start3).count();
        auto duration4 = duration_cast<milliseconds>(end4 - start4).count();
        auto duration5 = duration_cast<milliseconds>(end5 - start5).count();

//#pragma omp critical
        {
            writer.writeData(cnf.size(), numLiterals(cnf),
                             unit_prop.size(), numLiterals(unit_prop), duration1,
                             pure_literal.size(), numLiterals(pure_literal), duration2,
                             vivify.size(), numLiterals(vivify), duration3,
                             new_comb.size(), numLiterals(new_comb), duration4,
                             new_comb1.size(), numLiterals(new_comb1), duration5);


        }
         */
    }
    return 0;
}
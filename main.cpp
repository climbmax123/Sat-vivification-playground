#include <iostream>
#include "CnfTester.h"
#include "Preprocessor.h"
#include "preprocessing.h"
#include <chrono>


/*
int main() {

  CDNF_formula input = {{-4,1},{2, 3},{3, -5},{-4, 1}, {-5, 3}};
  auto red = vivify(input);

  CNFTester tester(100,100);

  for(int i = 0; i < 10000; i++) {
    auto cnf = tester.generateCNF();
    auto new_cnf = unit_propagation(cnf);
    auto vivifed_cnf = vivify(cnf);

    std::cout << "unit-porp:" << std::endl;
    tester.measureDifference(new_cnf);

    std::cout << "vivify:" << std::endl;
    tester.measureDifference(vivifed_cnf);
    if (tester.testCNF(cnf)) {
      std::cout << "equal" << std::endl;
    } else {
      tester.saveCNF("/Users/chris/CLionProjects/Vivifcation/testcases");
      std::cout << "not equal" << std::endl;
      exit(1);
    }
  }

  return 0;
}


*/

int main() {
    CNFTester tester(500, 500);
    long long vivifyTime = 0;
    long long vivifyWithPureLitTime = 0;

    for(int i = 0; i < 100; i++) {
        std::cout << i <<std::endl;
        auto cnf = tester.generateCNF();

        auto start = std::chrono::high_resolution_clock::now();
        auto vivifiedCNF = vivify(cnf);
        auto end = std::chrono::high_resolution_clock::now();
        vivifyTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

        start = std::chrono::high_resolution_clock::now();
        auto vivifiedWithPureLitCNF = vivify_with_pure_lit(cnf);
        end = std::chrono::high_resolution_clock::now();
        vivifyWithPureLitTime += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    double avgVivifyTime = vivifyTime / 10000.0;
    double avgVivifyWithPureLitTime = vivifyWithPureLitTime / 10000.0;

    std::cout << "Durchschnittliche Laufzeit von vivify: " << avgVivifyTime << " Mikrosekunden" << std::endl;
    std::cout << "Durchschnittliche Laufzeit von vivify_with_pure_lit: " << avgVivifyWithPureLitTime << " Mikrosekunden" << std::endl;
    std::cout << "Durchschnittlicher Laufzeitunterschied: " << (avgVivifyWithPureLitTime - avgVivifyTime) << " Mikrosekunden" << std::endl;

    return 0;
}
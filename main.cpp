#include <iostream>
#include "CnfTester.h"
#include "Preprocessor.h"
#include "preprocessing.h"

int main() {
  CNFTester tester(5,5);


  for(int i = 0; i < 5; i++) {
    auto cnf = reduced1_vivify(tester.generateCNF());
    //auto newcnf = unit_propagation(cnf);

    //tester.measureDifference(cnf);
    if (tester.testCNF(cnf)) {
      std::cout << "equal" << std::endl;
    } else {
      tester.saveCNF("/Users/chris/CLionProjects/Vivifcation/testcases");
      tester.printOriginalCNF();
      tester.printCNF(cnf);
      std::cout << "not equal" << std::endl;
      exit(1);
    }
  }

  return 0;
}



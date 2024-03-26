#include <iostream>
#include "CnfTester.h"
#include "Preprocessor.h"
#include "preprocessing.h"

int main() {

  CDNF_formula input = {{-1,-2,3,-4},{-1, -2, -3},{-1, 3, -4},{-1, -2, 4}};
  auto red = reduced1_vivify(input);

  CNFTester tester(50,50);

  std::cout << "End result: \t";
  tester.printCNF(red);


  for(int i = 0; i < 500; i++) {
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



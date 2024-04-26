//
// Created by Christofer Held on 01.03.24.
//

#ifndef VIVIFCATION__CNFTESTER_H_
#define VIVIFCATION__CNFTESTER_H_

//
// Created by Christofer Held on 01.03.24.
//

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <set>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>

#include "types.h"

class CNFTester {
 private:
  int num_variables;
  int num_clauses;

  CDNF_formula original_cnf;

  void saveCNFToFile(const std::string& filePath, const CDNF_formula& cnf);

  bool callMinisat(const CDNF_formula& cnf);

  static std::string generateLiteral(int variable);

  std::string generateUUID(const std::string& input_path);


 public:
  CNFTester(int variables, int clauses);

  CDNF_formula generateCNF();

  bool testCNF(const CDNF_formula & adapted_cnf);

  void printOriginalCNF();

  static void printCNF(const CDNF_formula& cnf);

  void measureDifference(const CDNF_formula& cnf);

  void saveCNF(const std::string& path);

  void loadCNF(const std::string& path);

};

#endif //VIVIFCATION__CNFTESTER_H_

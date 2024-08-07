//
// Created by Christofer Held on 02.06.24.
//

#ifndef VIVIFCATION_COMBINEDPREPROCESSING_H
#define VIVIFCATION_COMBINEDPREPROCESSING_H

#include "types.h"

#include <unordered_map>
#include <vector>
#include <iostream>
#include <chrono>
#include <set>


namespace combined {


    void pureLiteralElimination(CDNF_formula &formula, std::unordered_map<int, int> &mapping);

    void unit_propagation(CDNF_formula &cnf, std::unordered_map<int, int> &mapping);

    void combined_methode(CDNF_formula& cnf, int timeLimitInSeconds = -1);

    std::unordered_map<int, int> creat_mapping(CDNF_formula &formula);


} // combined

#endif //VIVIFCATION_COMBINEDPREPROCESSING_H

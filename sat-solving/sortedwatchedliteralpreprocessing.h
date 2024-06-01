//
// Created by Christofer Held on 21.05.24.
//

#ifndef VIVIFCATION_SORTEDWATCHEDLITERALPREPROCESSING_H
#define VIVIFCATION_SORTEDWATCHEDLITERALPREPROCESSING_H

#include "types.h"
#include "CSVWriter.h"


#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>
#include <chrono>

using namespace std::chrono;

namespace sorted {

    void vivify_with_sorted_pure_lit(CDNF_formula &cnf, int timeLimitInSeconds = -1);


} // sorted

#endif //VIVIFCATION_SORTEDWATCHEDLITERALPREPROCESSING_H

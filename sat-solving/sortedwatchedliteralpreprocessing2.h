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


// Sorted is the same as normal vivify with the only differance that we changed the way we keep track of the mapping
// As updating the mapping only takes O(n) time and sorting takes O(n log n) we rather want to avoid resorts.

// Furthermore, we can keep track of non reduce able literals and sort them to the front. As they won't help us. to find pures

// a literal is non reduce able if there is for positive and for negative occurrence a clause where it's non redundant.

namespace sorted2 {

    void vivify_with_sorted_pure_lit(CDNF_formula &cnf, int timeLimitInSeconds = -1);


} // sorted

#endif //VIVIFCATION_SORTEDWATCHEDLITERALPREPROCESSING_H

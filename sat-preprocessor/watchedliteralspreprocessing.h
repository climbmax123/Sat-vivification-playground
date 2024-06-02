//
// Created by Christofer Held on 25.04.24.
//

#ifndef VIVIFCATION_WATCHEDLITERALSPREPROCESSING_H
#define VIVIFCATION_WATCHEDLITERALSPREPROCESSING_H

#include "types.h"

#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>
#include <chrono>


using namespace std::chrono;

namespace watched_literals {

    void unit_propagation(CDNF_formula &cnf, int timeLimitInSeconds = -1);

    void vivify(CDNF_formula &cnf, int timeLimitInSeconds = -1);
}

#endif

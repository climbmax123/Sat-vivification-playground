//
// Created by Christofer Held on 15.07.24.
//

#ifndef VIVIFCATION_SAT_VIVIFY_H
#define VIVIFCATION_SAT_VIVIFY_H

#include "types.h"

#include <map>
#include <set>
#include <vector>
#include <iostream>
#include <chrono>


using namespace std::chrono;

namespace satvivify {

    void watched_literals_vivify(QBF &qbf, int timeLimitInSeconds);

}

#endif //VIVIFCATION_SAT_VIVIFY_H

//
// Created by christofer on 26.04.24.
//

#ifndef VIVIFCATION_TYPES_H
#define VIVIFCATION_TYPES_H

#include <vector>
#include <unordered_map>

struct QBF {
    std::unordered_map<int, bool> quantifierTypeIsExists; // how the values are is needed for compute
    std::vector<int> quantifierOrder; // how we order the quantifier not needed while compute
    std::vector<std::vector<int>> formula;
};

#endif //VIVIFCATION_TYPES_H

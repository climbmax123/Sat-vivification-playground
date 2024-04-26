//
// Created by christofer on 26.04.24.
//

#ifndef VIVIFCATION_TYPES_H
#define VIVIFCATION_TYPES_H

#include <vector>
#include <unordered_map>

enum QuantifierType
{
    EXISTS = -1,
    UNDEF = 0,
    FORALL = 1
};

struct QBF {
    std::vector<std::pair<QuantifierType, unsigned int>> quantifiers;
    std::vector<std::vector<int>> formula;
};

#endif //VIVIFCATION_TYPES_H

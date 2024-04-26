//
// Created by Christofer Held on 12.04.24.
//

#ifndef VIVIFCATION_WATCHEDPREPROCESSING_H
#define VIVIFCATION_WATCHEDPREPROCESSING_H

#include "types.h"

#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>

namespace watched {

    void unit_propagation(CDNF_formula &cnf);

    void vivify(CDNF_formula &cnf);

    void vivify_with_pure_lit(CDNF_formula &cnf);
}
#endif //VIVIFCATION_WATCHEDPREPROCESSING_H

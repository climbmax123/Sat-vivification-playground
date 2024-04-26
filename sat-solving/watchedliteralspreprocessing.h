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


namespace watched_literals {

    void watched_literals_unit_propagation(CDNF_formula &cnf);

   // void vivify(CDNF_formula &cnf);

    //void vivify_with_pure_lit(CDNF_formula &cnf);

}

#endif

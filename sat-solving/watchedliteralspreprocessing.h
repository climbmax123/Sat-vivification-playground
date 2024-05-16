//
// Created by Christofer Held on 25.04.24.
//

#ifndef VIVIFCATION_WATCHEDLITERALSPREPROCESSING_H
#define VIVIFCATION_WATCHEDLITERALSPREPROCESSING_H

#include "types.h"
#include "CSVWriter.h"


#include <unordered_map>
#include <set>
#include <vector>
#include <iostream>
#include <chrono>


using namespace std::chrono;

namespace watched_literals {

    void watched_literals_unit_propagation(CDNF_formula &cnf);

    void vivify(CDNF_formula &cnf, int global_count, CSVWriter &writer);

    void vivify_with_pure_lit(CDNF_formula &cnf, int global_count, CSVWriter &writer, CSVWriter& pure_writer);

    void vivify_with_sorted_pure_lit(CDNF_formula &cnf, int global_count, CSVWriter &writer, CSVWriter& pure_writer);

}

#endif

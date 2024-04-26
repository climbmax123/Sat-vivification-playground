//
// Created by Christofer Held on 12.04.24.
//

#ifndef VIVIFCATION_FASTPREPROCESSING_H
#define VIVIFCATION_FASTPREPROCESSING_H

#include "types.h"

namespace fast {

    void UP(CDNF_formula &cnf, std::vector<std::pair<int, int> > &found_units);

    void PureLiteralElimination(CDNF_formula &formula);

    void Vivify(CDNF_formula &cnf);

    void VivifyWithPureLit(CDNF_formula &cnf);

}
#endif //VIVIFCATION_FASTPREPROCESSING_H

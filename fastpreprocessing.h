//
// Created by Christofer Held on 12.04.24.
//

#ifndef VIVIFCATION_FASTPREPROCESSING_H
#define VIVIFCATION_FASTPREPROCESSING_H

#include "types.h"


void FastUP(CDNF_formula& cnf, std::vector<std::pair<int,int> >& found_units);

void FastPureLiteralElimination(CDNF_formula& formula);

void FastVivify(CDNF_formula& cnf);

void FastVivifyWithPureLit(CDNF_formula& cnf);

#endif //VIVIFCATION_FASTPREPROCESSING_H

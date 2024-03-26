//
// Created by Christofer Held on 07.03.24.
//

#ifndef VIVIFCATION__PREPROCESSING_H_
#define VIVIFCATION__PREPROCESSING_H_

#include "types.h"


CDNF_formula unit_propagation(CDNF_formula cnf);

CDNF_formula vivify(CDNF_formula cnf);

CDNF_formula vivify2(CDNF_formula cnf);

CDNF_formula reduced1_vivify(CDNF_formula cnf);

CDNF_formula reduced2_vivify(CDNF_formula cnf);

CDNF_formula UP_reduce(CDNF_formula cnf);

#endif //VIVIFCATION__PREPROCESSING_H_

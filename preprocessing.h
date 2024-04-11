//
// Created by Christofer Held on 07.03.24.
//

#ifndef VIVIFCATION__PREPROCESSING_H_
#define VIVIFCATION__PREPROCESSING_H_

#include "types.h"


CDNF_formula unit_propagation(CDNF_formula cnf);

CDNF_formula vivify(CDNF_formula cnf);

CDNF_formula vivify_with_pure_lit(CDNF_formula cnf);

CDNF_formula vivify_with_pure_lit2(CDNF_formula cnf);


#endif //VIVIFCATION__PREPROCESSING_H_

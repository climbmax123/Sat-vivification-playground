//
// Created by christofer on 26.04.24.
//

#ifndef VIVIFCATION_VIVIFY_H
#define VIVIFCATION_VIVIFY_H

#include "types.h"
#include <set>

namespace vivify{
    void watched_literals_unit_propagation(QBF &qbf);

    void watched_literals_vivify(QBF &qbf);
};
#endif //VIVIFCATION_VIVIFY_H

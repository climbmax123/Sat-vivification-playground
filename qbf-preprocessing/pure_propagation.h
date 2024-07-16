//
// Created by Christofer Held on 27.06.24.
//

#ifndef VIVIFCATION_PURE_PROPAGATION_H
#define VIVIFCATION_PURE_PROPAGATION_H

#include "types.h"

namespace pure {

    void pure_propagation_with_universals(QBF& qbf, int timeLimitInSeconds);

    void pure_propagation_without_universals(QBF& qbf, int timeLimitInSeconds);

} // pure

#endif //VIVIFCATION_PURE_PROPAGATION_H

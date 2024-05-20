//
// Created by Christofer Held on 18.05.24.
//

#ifndef VIVIFCATION_CNFFORMULA_H
#define VIVIFCATION_CNFFORMULA_H
#include <vector>

// The goal of this approach is to build a data structure for faster working with

class CnfFormula {

private:
    // We work with the following structure
    //      c1  c2  c3  c4  ...
    //  l1  1   0   1   0
    // -l1  0   1   0   0
    //  l2  1   1   0   1
    // -l2  0   0   1   0
    //  ...
    std::vector<std::vector<bool>> transposed;

    // We work with the following structure
    //      l1  -l1  l2  -l2  ...
    //  c1   1    0   1   0
    //  c2   0    1   1   0
    //  c3   1    0   0   1
    //  c4   0    0   1   0
    //  ...
    std::vector<std::vector<bool>> normal;

    // Note: In our Program we need to always ensure that they are equal (Transposed to each other).

    // We now can work with optimized and perfectly parallel operations
    // Pure Literal Elimination:
    // To remove Pures: Transposed x (1)^(num_clauses) = calculate occupancy vector
    // Calculate:
    //      c1  c2  c3  c4  ...
    //   l1  1   0   1   0              1
    //  -l1  0   1   0   0         X    1
    //   l2  1   1   0   1              1
    //  -l2  0   0   1   0   ...        1
    //  ...                            ...
    // This can be done in with a perfect cache miss rate and on a Tensor Unit
    // Then you then can find the pures simply by finding the 0 values in the vector.
    void pureElimination() {
        return;
    }

    // We can do Unit propagation the following way.







    std::vector<bool> sat;

    std::vector<bool> assigned;

    std::vector<std::vector<bool>> create_transpose(const std::vector<std::vector<bool>>& matrix);



};


#endif //VIVIFCATION_CNFFORMULA_H

//
// Created by christofer on 26.04.24.
//

#include "input-parser.h"
#include "vivify.h"
#include "pure_propagation.h"
#include "unit_propagation.h"

int main(int argc, char* argv[]) {
    if (argc > 2) {
        QBF qbf = parseQDIMACSFromFile(argv[1]);

        printQBF(qbf);
        pure::pure_propagation_with_universals(qbf);
        printQBF(qbf);
        writeQDIMACS(qbf, argv[2]);

        return 0;
    }
    return 1;
}
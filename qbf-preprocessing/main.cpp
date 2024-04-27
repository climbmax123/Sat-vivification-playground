//
// Created by christofer on 26.04.24.
//

#include "input-parser.h"


int main(int argc, char* argv[]) {
    if (argc > 2) {
        // Parse from file
        QBF qbf = parseQDIMACSFromFile(argv[1]);
        // Process the parsed QBF...
        printQBF(qbf);
        writeQDIMACS(qbf, argv[2]);

        return 0;
    }
    return 1;
}
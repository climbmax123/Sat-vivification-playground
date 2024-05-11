//
// Created by christofer on 26.04.24.
//

#include "input-parser.h"
#include "vivify.h"

int main(int argc, char* argv[]) {
    if (argc > 2) {
        // Parse from file
        std::cout << "here4" << std::endl;
        QBF qbf = parseQDIMACSFromFile(argv[1]);
        std::cout << "here5" << std::endl;
        // Process the parsed QBF...
        printQBF(qbf);
        vivify::watched_literals_unit_propagation(qbf);
        printQBF(qbf);
        writeQDIMACS(qbf, argv[2]);

        return 0;
    }
    return 1;
}
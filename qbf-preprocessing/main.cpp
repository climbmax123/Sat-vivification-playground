//
// Created by christofer on 26.04.24.
//

#include "input-parser.h"
#include "vivify.h"
#include "pure_propagation.h"
#include "unit_propagation.h"
#include "sat_vivify.h"

#include <getopt.h>

int main(int argc, char *argv[]) {
    int opt;
    enum {
        PURE, UNIT, VIVIFY, UNIT_WITH_UR
    } mode;

    const char *options = "u:v:p";
    while ((opt = getopt(argc, argv, options)) != -1) {
        switch (opt) {
            case 'u':
                mode = UNIT;
                break;
            case 'v':
                mode = VIVIFY;
                break;
            case 'p':
                mode = PURE;
                break;
            default:
                std::cerr << "Invalid option provided." << std::endl;
                return 1;
        }
    }

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [-u|-v|-p] <inputfile> <outputfile>" << std::endl;
        return 1;
    }

    int timeLimitInSeconds = 30*60;
    QBF qbf = parseQDIMACSFromFile(argv[2]);
    switch (mode) {
        case UNIT:
            std::cout << "Start Unit Propagation" << std::endl;
            unit::watched_literals_unit_propagation_with_ur(qbf, timeLimitInSeconds);
            break;
        case VIVIFY:
            std::cout << "Start Vivify" << std::endl;
            satvivify::watched_literals_vivify(qbf, timeLimitInSeconds);
            break;
        case PURE:
            std::cout << "Start Pure elimination" << std::endl;
            pure::pure_propagation_with_universals(qbf, timeLimitInSeconds);
            break;
        default:
            std::cerr << "wrong input" << std::endl;
            break;
    }
    writeQDIMACS(qbf, argv[3]);

    return 0;

}
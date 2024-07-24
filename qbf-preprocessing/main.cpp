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
        PURE, UNIT, VIVIFY, UNIT_WITH_UR, COMBI
    } mode;

    const char *options = "u:v:p:c";
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
            case 'c':
                mode = COMBI;
                break;
            default:
                std::cerr << "Invalid option provided." << std::endl;
                return 1;
        }
    }

    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " [-u|-v|-p|-c] <inputfile> <outputfile>" << std::endl;
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
        case COMBI:
            std::cout << "Start Combi appoach" << std::endl;
            unit::watched_literals_unit_propagation_with_ur(qbf, 2*60);
            satvivify::watched_literals_vivify(qbf, 25*60);
            unit::watched_literals_unit_propagation_with_ur(qbf, 2*60);
            pure::pure_propagation_with_universals(qbf, 1*60);
            break; // (2+25+2+1)*60 = 1800
        default:
            std::cerr << "wrong input" << std::endl;
            break;
    }
    writeQDIMACS(qbf, argv[3]);

    return 0;

}
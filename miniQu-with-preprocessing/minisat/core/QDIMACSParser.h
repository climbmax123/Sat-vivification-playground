//
// Created by Christofer Held on 23.04.24.
//

#ifndef MINISAT_QDIMACSPARSER_H
#define MINISAT_QDIMACSPARSER_H

#include <vector>
#include <string>

#include "QBFParser.h"
#include "Preprocessor.h"
#include "Solver.h"

using std::vector;
using std::string;

class QDIMACSParser : virtual public QBFParser {
public:
    QDIMACSParser(const string& filename);
    void initSolver(Minisat::Solver& solver);
    string getOriginalName(int alias) const;
protected:
    QDIMACSParser();
    std::vector<GatePolarity> polarities;

};


#endif //MINISAT_QDIMACSPARSER_H

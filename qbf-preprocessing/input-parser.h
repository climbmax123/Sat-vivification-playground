//
// Created by christofer on 26.04.24.
//

#ifndef VIVIFCATION_INPUT_PARSER_H
#define VIVIFCATION_INPUT_PARSER_H

#include "types.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdexcept>

void printQBF(QBF& qbf);

QBF parseQDIMACSFromFile(const std::string& filename);

void writeQDIMACS(QBF& qbf, const std::string& filename);

#endif //VIVIFCATION_INPUT_PARSER_H

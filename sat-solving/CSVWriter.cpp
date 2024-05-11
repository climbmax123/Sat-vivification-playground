//
// Created by Christofer Held on 11.05.24.
//

#include "CSVWriter.h"

CSVWriter::CSVWriter(const std::string &filename) : filename(filename) {
    file.open(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    // Write the CSV header
    file << "num_clauses,num_literals,red_unit_clauses,red_unit_literals,"
            "red_pure_clauses,red_pure_literals,red_comb_clauses,red_comb_literals\n";
}

CSVWriter::~CSVWriter() {
    if (file.is_open()) {
        file.close();
    }
}

void CSVWriter::writeData(int num_clauses, int num_literals, int red_unit_clauses, int red_unit_literals,
                          int red_pure_clauses, int red_pure_literals, int red_comb_clauses, int red_comb_literals) {
    if (!file.is_open()) {
        std::cerr << "File is not open for writing." << std::endl;
        return;
    }
    file << num_clauses << "," << num_literals << ","
         << red_unit_clauses << "," << red_unit_literals << ","
         << red_pure_clauses << "," << red_pure_literals << ","
         << red_comb_clauses << "," << red_comb_literals << "\n";
}

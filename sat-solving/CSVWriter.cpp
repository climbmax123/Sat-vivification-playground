//
// Created by Christofer Held on 11.05.24.
//

#include "CSVWriter.h"

CSVWriter::CSVWriter(const std::string &filename, std::string head) : filename(filename) {
    file.open(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    // Write the CSV header
    file << head << std::endl;
}

CSVWriter::~CSVWriter() {
    if (file.is_open()) {
        file.close();
    }
}

void CSVWriter::writeData_runtime(int num_clauses, int num_literals,
                          int red_unit_clauses, int red_unit_literals, long long unit_duration,
                          int red_pure_clauses, int red_pure_literals, long long pure_duration,
                          int red_vivify_clauses, int red_vivify_literals, long long vivify_duration,
                          int red_comb_up_clauses, int red_comb_up_literals, long long up_duration,
                          int red_comb_vp_clauses, int red_comb_vp_literals, long long vp_duration) {
    if (!file.is_open()) {
        std::cerr << "File is not open for writing." << std::endl;
        return;
    }
    file << num_clauses << "," << num_literals << ","
         << red_unit_clauses << "," << red_unit_literals << "," << unit_duration << ","
         << red_pure_clauses << "," << red_pure_literals << "," << pure_duration << ","
            << red_vivify_clauses << "," << red_vivify_literals << "," << vivify_duration << ","
            << red_comb_up_clauses << "," << red_comb_up_literals << "," << up_duration << ","
            << red_comb_vp_clauses << "," << red_comb_vp_literals << "," << vp_duration << std::endl;
}


void CSVWriter::write_vivification_runtime(int count,  int step, int num_clauses, int changed_clauses, int total_num_vars, int num_literals, long long duration) {
    if (!file.is_open()) {
        std::cerr << "File is not open for writing." << std::endl;
        return;
    }
    file << count << "," << step << ","
         << num_clauses << "," << changed_clauses << "," << total_num_vars << ","
         << num_literals << ","<< duration << std::endl;
}

void CSVWriter::write_pure(int count, int step, int interation, int found_pures, int reduced_clauses_count,
                           long long int duration) {

    if (!file.is_open()) {
        std::cerr << "File is not open for writing." << std::endl;
        return;
    }
    file << count << "," << step << ","
         << interation << "," << found_pures << "," << reduced_clauses_count << ","
         << duration << std::endl;
}

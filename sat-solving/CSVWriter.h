//
// Created by Christofer Held on 11.05.24.
//

#ifndef VIVIFCATION_CSVWRITER_H
#define VIVIFCATION_CSVWRITER_H

#include <fstream>
#include <iostream>
#include <string>


class CSVWriter {
private:
    std::ofstream file;
    std::string filename;

public:
    // Constructor
    CSVWriter(const std::string &filename, std::string head);

    // Destructor
    ~CSVWriter();

    // Function to write a line to the CSV file
    void writeData_runtime(int num_clauses, int num_literals,
                           int red_unit_clauses, int red_unit_literals, long long unit_duration,
                           int red_pure_clauses, int red_pure_literals, long long pure_duration,
                           int red_vivify_clauses, int red_vivify_literals, long long vivify_duration,
                           int red_comb_up_clauses, int red_comb_up_literals, long long up_duration,
                           int red_comb_vp_clauses, int red_comb_vp_literals, long long vp_duration);


    void write_vivification_runtime(int count, int step, int num_clauses, int changed_clauses, int total_num_vars,
                                    int num_literals, long long duration);


    void write_pure(int count, int step, int interation, int found_pures, int reduced_clauses_count, long long duration);

};


#endif //VIVIFCATION_CSVWRITER_H

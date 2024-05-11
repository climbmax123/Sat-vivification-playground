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
    CSVWriter(const std::string& filename);

    // Destructor
    ~CSVWriter();

    // Function to write a line to the CSV file
    void writeData(int num_clauses, int num_literals, int red_unit_clauses, int red_unit_literals,
                   int red_pure_clauses, int red_pure_literals, int red_comb_clauses, int red_comb_literals);
};



#endif //VIVIFCATION_CSVWRITER_H

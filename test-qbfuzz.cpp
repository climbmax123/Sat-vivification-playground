//
// Created by Christofer Held on 11.04.24.
//
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    try {
        std::string qbfFormula = exec("python3 qbfuzz.py");
        std::cout << "Generated QBF Formula: \n" << qbfFormula << std::endl;
        // Process the QBF formula as needed
    } catch (const std::exception& e) {
        std::cerr << "Error executing qbfuzz.py: " << e.what() << std::endl;
    }
    return 0;
}

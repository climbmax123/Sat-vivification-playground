//
// Created by Christofer Held on 18.05.24.
//

// For this Project we will test out a new methode of working with S
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <ctime>
#include <chrono>

template <typename T>
T generateRandomValue();

// Spezialisierung der Funktion für int
template <>
int generateRandomValue<int>() {
    return rand() % 100;
}

template <>
short generateRandomValue<short>() {
    return static_cast<short >(rand() % 100);
}

template <>
bool generateRandomValue<bool>() {
    return static_cast<bool>(rand() % 2 == 0);
}

template <typename T>
std::vector<T> createRandomArray(size_t size) {
    std::vector<T> array(size);
    for (size_t i = 0; i < size; ++i) {
        array[i] = generateRandomValue<T>();
    }
    return array;
}

template <typename T>
std::vector<std::vector<T>> createRadomMatrix(size_t height, size_t width){
    std::vector<std::vector<T>> array(height);
    for(size_t i = 0; i < height; ++i){
        array[i] = createRandomArray<T>(width);
    }
    return array;
}


template <typename T>
std::vector<T> mat_vec_mul(const std::vector<std::vector<T>>& matrix, const std::vector<T>& vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<T> result(height, 0);

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            result[i] += matrix[i][j] * vec[j];
        }
    }

    return result;
}

std::vector<bool> mat_vec_mul_bool(const std::vector<std::vector<bool>>& matrix, const std::vector<bool>& vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<bool> result(height, false);

    for (size_t i = 0; i < height; ++i) {
        for (size_t j = 0; j < width; ++j) {
            if (matrix[i][j] && vec[j]) {
                result[i] = true;
                break; // Frühzeitiger Abbruch, da mindestens ein Element wahr ist
            }
        }
    }

    return result;
}

template <typename T>
std::vector<T> mat_vec_mul2(const std::vector<std::vector<T>>& matrix, const std::vector<T>& vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<T> result(height, 0);

    for (size_t j = 0; j < width; ++j) {
        T vec_j = vec[j];
        for (size_t i = 0; i < height; ++i) {
            result[i] += matrix[i][j] * vec_j;
        }
    }

    return result;
}

int main(){
    auto matrix1 = createRadomMatrix<int>(10000,10000);
    auto vector1 = std::vector<int>(10000, 1);

    double time1 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul(matrix1, vector1);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time1 += duration.count();
    }

    std::cout << "Laufzeit <int> für 10: " << time1 << " ms" << std::endl;

    auto matrix2 = createRadomMatrix<short>(10000,10000);
    auto vector2 = std::vector<short>(10000, 1);

    double time2 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul(matrix2, vector2);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time2 += duration.count();
    }

    std::cout << "Laufzeit für <short> 10: " << time2 << " ms" << std::endl;

    auto matrix3 = createRadomMatrix<bool>(10000,10000);
    auto vector3 = std::vector<bool>(10000, 1);

    double time3 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul_bool(matrix3, vector3);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time3 += duration.count();
    }

    std::cout << "Laufzeit für <bool> 10: " << time3 << " ms" << std::endl;

    std::cout << "\nmethode 2\n" << std::endl;

    auto matrix4 = createRadomMatrix<int>(10000,10000);
    auto vector4 = std::vector<int>(10000, 1);

    double time4 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul(matrix4, vector4);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time4 += duration.count();
    }

    std::cout << "Laufzeit <int> für 10: " << time4 << " ms" << std::endl;

    auto matrix5 = createRadomMatrix<short>(10000,10000);
    auto vector5 = std::vector<short>(10000, 1);

    double time5 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul(matrix5, vector5);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time5 += duration.count();
    }

    std::cout << "Laufzeit für <short> 10: " << time5 << " ms" << std::endl;

    auto matrix6 = createRadomMatrix<bool>(10000,10000);
    auto vector6 = std::vector<bool>(10000, 1);

    double time6 = 0;
    for(int i = 0; i < 10; i++){
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = mat_vec_mul_bool(matrix6, vector6);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time6 += duration.count();
    }

    std::cout << "Laufzeit für <bool> 10: " << time6 << " ms" << std::endl;




    return 0;
}
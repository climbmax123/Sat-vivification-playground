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
#include <ranges>
#include <execution>
#include <immintrin.h>

template<typename T>
T generateRandomValue();

// Spezialisierung der Funktion für int
template<>
int generateRandomValue<int>() {
    return rand() % 100;
}

template<>
int8_t generateRandomValue<int8_t>() {
    return rand() % 100 == 0 ? 1 : 0;
}

template<>
short generateRandomValue<short>() {
    return static_cast<short >(rand() % 100);
}

template<>
bool generateRandomValue<bool>() {
    return static_cast<bool>(rand() % 5000 == 0);
}

template<typename T>
std::vector<T> createRandomArray(size_t size) {
    std::vector<T> array(size);
    for (size_t i = 0; i < size; ++i) {
        array[i] = generateRandomValue<T>();
    }
    return array;
}

template<typename T>
std::vector<std::vector<T>> createRadomMatrix(size_t height, size_t width) {
    std::vector<std::vector<T>> array(height);
    for (size_t i = 0; i < height; ++i) {
        array[i] = createRandomArray<T>(width);
    }
    return array;
}


template<typename T>
std::vector<T> mat_vec_mul(const std::vector<std::vector<T>> &matrix, const std::vector<T> &vec) {
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

std::vector<bool> mat_vec_mul_bool(const std::vector<std::vector<bool>> &matrix, const std::vector<bool> &vec) {
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


#include <vector>
#include <immintrin.h> // Für AVX-512-Instruktionen

#include <vector>
#include <immintrin.h> // Für AVX-512-Instruktionen

std::vector<bool> mat_vec_mul_bool2(const std::vector<std::vector<bool>> &matrix, const std::vector<bool> &vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<bool> result(height, false);

    // Konvertiere den Vektor zu Masken
    std::vector<__mmask64> vec_masks((width + 63) / 64, 0);
    for (size_t j = 0; j < width; ++j) {
        if (vec[j]) {
            vec_masks[j / 64] |= (1ULL << (j % 64));
        }
    }

    for (size_t i = 0; i < height; ++i) {
        bool found = false;
        for (size_t block = 0; block < vec_masks.size(); ++block) {
            __mmask64 mat_mask = 0;
            size_t start = block * 64;
            size_t end = std::min(start + 64, width);
            for (size_t j = start; j < end; ++j) {
                if (matrix[i][j]) {
                    mat_mask |= (1ULL << (j % 64));
                }
            }

            if (vec_masks[block] & mat_mask) {
                result[i] = true;
                found = true;
                break; // Frühzeitiger Abbruch, da mindestens ein Element wahr ist
            }
        }
        if (!found) {
            result[i] = false;
        }
    }

    return result;
}

#include <vector>
#include <ranges>
#include <immintrin.h> // Für AVX2-Instruktionen



std::vector<int> mat_vec_mul_int_2(const std::vector<std::vector<int>> &matrix, const std::vector<int> &vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<int> result(height, 0);

    for (size_t j = 0; j < width; ++j) {
        __m256i vec_j_avx = _mm256_set1_epi32(vec[j]);
        size_t i = 0;

        // Verarbeite die Hauptschleife mit AVX2
        for (; i + 8 <= height; i += 8) {
            __m256i mat_col_avx = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(&matrix[i][j]));
            __m256i result_avx = _mm256_loadu_si256(reinterpret_cast<__m256i *>(&result[i]));
            result_avx = _mm256_add_epi32(result_avx, _mm256_mullo_epi32(mat_col_avx, vec_j_avx));
            _mm256_storeu_si256(reinterpret_cast<__m256i *>(&result[i]), result_avx);
        }

        // Verarbeite die restlichen Elemente
        for (; i < height; ++i) {
            result[i] += matrix[i][j] * vec[j];
        }
    }

    return result;
}

std::vector<std::vector<int>> multiplyMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_cols = B[0].size();

    std::vector<std::vector<int>> result(A_rows, std::vector<int>(B_cols, 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t j = 0; j < B_cols; ++j) {
            for (size_t k = 0; k < A_cols; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}


std::vector<std::vector<short>> multiplyMatrices(const std::vector<std::vector<short>>& A, const std::vector<std::vector<short>>& B) {
    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_cols = B[0].size();

    std::vector<std::vector<short>> result(A_rows, std::vector<short>(B_cols, 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t j = 0; j < B_cols; ++j) {
            for (size_t k = 0; k < A_cols; ++k) {
                result[i][j] += A[i][k] * B[k][j];
            }
        }
    }

    return result;
}

std::vector<std::vector<int>> multiplyMatricesAVX512(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_cols = B[0].size();

    std::vector<std::vector<int>> result(A_rows, std::vector<int>(B_cols, 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t k = 0; k < A_cols; ++k) {
            __m512i a_val = _mm512_set1_epi32(A[i][k]);
            for (size_t j = 0; j < B_cols; j += 16) {
                int remainder = B_cols - j < 16 ? B_cols - j : 16;
                __m512i b_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&B[k][j]));
                __m512i res_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&result[i][j]));

                __m512i mul_vals = _mm512_mullo_epi32(a_val, b_vals);
                res_vals = _mm512_add_epi32(res_vals, mul_vals);

                // Mask to handle the remainder of elements when B_cols is not a multiple of 16
                __mmask16 mask = (1 << remainder) - 1;
                _mm512_mask_storeu_epi32(reinterpret_cast<__m512i*>(&result[i][j]), mask, res_vals);
            }
        }
    }

    return result;
}

#include <vector>
#include <immintrin.h> // Für AVX2-Instruktionen

std::vector<int> mat_vec_mul_short(const std::vector<std::vector<short>> &matrix, const std::vector<short> &vec) {
    size_t height = matrix.size();
    size_t width = vec.size();
    std::vector<int> result(height, 0);

    for (size_t j = 0; j < width; ++j) {
        __m256i vec_j_avx = _mm256_set1_epi16(vec[j]);
        size_t i = 0;

        // Verarbeite die Hauptschleife mit AVX2
        for (; i + 16 <= height; i += 16) {
            __m256i mat_col_avx = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(&matrix[i][j]));
            __m256i result_avx = _mm256_loadu_si256(reinterpret_cast<__m256i *>(&result[i]));
            __m256i mul_res = _mm256_mullo_epi16(mat_col_avx, vec_j_avx);
            result_avx = _mm256_add_epi16(result_avx, mul_res);
            _mm256_storeu_si256(reinterpret_cast<__m256i *>(&result[i]), result_avx);
        }

        // Verarbeite die restlichen Elemente
        for (; i < height; ++i) {
            result[i] += matrix[i][j] * vec[j];
        }
    }

    return result;
}



std::vector<std::vector<int>> multiplyMatricesAVX512_8bit(const std::vector<std::vector<int8_t>>& A, const std::vector<std::vector<int8_t>>& B) {
    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_cols = B[0].size();

    std::vector<std::vector<int>> result(A_rows, std::vector<int>(B_cols, 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t k = 0; k < A_cols; ++k) {
            __m512i a_val = _mm512_set1_epi8(A[i][k]);
            for (size_t j = 0; j < B_cols; j += 64) {
                int remainder = B_cols - j < 64 ? B_cols - j : 64;
                __m512i b_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&B[k][j]));

                // Unpack and multiply, then accumulate
                __m512i res_vals_low = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&result[i][j]));
                __m512i res_vals_high = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&result[i][j + 32]));

                __m512i a_val_low = _mm512_unpacklo_epi8(a_val, _mm512_setzero_si512());
                __m512i a_val_high = _mm512_unpackhi_epi8(a_val, _mm512_setzero_si512());

                __m512i b_vals_low = _mm512_unpacklo_epi8(b_vals, _mm512_setzero_si512());
                __m512i b_vals_high = _mm512_unpackhi_epi8(b_vals, _mm512_setzero_si512());

                __m512i mul_vals_low = _mm512_madd_epi16(a_val_low, b_vals_low);
                __m512i mul_vals_high = _mm512_madd_epi16(a_val_high, b_vals_high);

                res_vals_low = _mm512_add_epi32(res_vals_low, mul_vals_low);
                res_vals_high = _mm512_add_epi32(res_vals_high, mul_vals_high);

                // Mask to handle the remainder of elements when B_cols is not a multiple of 64
                __mmask64 mask = (1ULL << remainder) - 1;
                _mm512_mask_storeu_epi32(reinterpret_cast<__m512i*>(&result[i][j]), mask, res_vals_low);
                if (remainder > 32) {
                    _mm512_mask_storeu_epi32(reinterpret_cast<__m512i*>(&result[i][j + 32]), mask >> 32, res_vals_high);
                }
            }
        }
    }

    return result;
}



#include <immintrin.h>
#include <vector>
#include <stdexcept>

std::vector<std::vector<int>> multiplyMatricesAVX512_8bit_Transposed(
        const std::vector<std::vector<int8_t>>& A,
        const std::vector<std::vector<int8_t>>& B_transposed) {

    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_rows = B_transposed.size();  // Originally B_cols in non-transposed

    // Ensure that the matrix dimensions are compatible for multiplication
    if (A_cols != B_rows) {
        throw std::invalid_argument("Matrix dimensions must be compatible for multiplication.");
    }

    std::vector<std::vector<int>> result(A_rows, std::vector<int>(B_transposed[0].size(), 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t j = 0; j < B_transposed[0].size(); ++j) {
            __m512i sum = _mm512_setzero_si512();  // Initialize the sum for each element in result[i][j]

            for (size_t k = 0; k < A_cols; k += 64) {
                int block_size = std::min(A_cols - k, size_t(64));
                __m512i a_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&A[i][k]));
                __m512i b_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&B_transposed[j][k]));

                __m512i a_vals_low = _mm512_unpacklo_epi8(a_vals, _mm512_setzero_si512());
                __m512i a_vals_high = _mm512_unpackhi_epi8(a_vals, _mm512_setzero_si512());

                __m512i b_vals_low = _mm512_unpacklo_epi8(b_vals, _mm512_setzero_si512());
                __m512i b_vals_high = _mm512_unpackhi_epi8(b_vals, _mm512_setzero_si512());

                __m512i mul_low = _mm512_madd_epi16(a_vals_low, b_vals_low);
                __m512i mul_high = _mm512_madd_epi16(a_vals_high, b_vals_high);

                sum = _mm512_add_epi32(sum, mul_low);
                sum = _mm512_add_epi32(sum, mul_high);

                // Accumulate sums for the elements in the block
                if (block_size < 64) {
                    __mmask64 mask = (1ULL << block_size) - 1;
                    sum = _mm512_maskz_add_epi32(mask, sum, _mm512_setzero_si512());  // Mask sum to handle blocks smaller than 64
                }
            }
            // Store the final summed value for each element in result[i][j]
            int res_value = _mm512_reduce_add_epi32(sum);
            result[i][j] = res_value;
        }
    }

    return result;
}

std::vector<std::vector<int>> multiplyMatricesAVX512_8bit_custom_block_size(
        const std::vector<std::vector<int8_t>>& A,
        const std::vector<std::vector<int8_t>>& B_transposed,
        int block_size) {

    size_t A_rows = A.size();
    size_t A_cols = A[0].size();
    size_t B_rows = B_transposed.size();  // Originally B_cols in non-transposed

    // Ensure that the matrix dimensions are compatible for multiplication
    if (A_cols != B_rows) {
        throw std::invalid_argument("Matrix dimensions must be compatible for multiplication.");
    }

    std::vector<std::vector<int>> result(A_rows, std::vector<int>(B_transposed[0].size(), 0));

    for (size_t i = 0; i < A_rows; ++i) {
        for (size_t j = 0; j < B_transposed[0].size(); ++j) {
            __m512i sum = _mm512_setzero_si512();  // Initialize the sum for each element in result[i][j]

            for (size_t k = 0; k < A_cols; k += block_size) {
                int b = std::min(A_cols - k, size_t(block_size));
                __m512i a_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&A[i][k]));
                __m512i b_vals = _mm512_loadu_si512(reinterpret_cast<const __m512i*>(&B_transposed[j][k]));

                __m512i a_vals_low = _mm512_unpacklo_epi8(a_vals, _mm512_setzero_si512());
                __m512i a_vals_high = _mm512_unpackhi_epi8(a_vals, _mm512_setzero_si512());

                __m512i b_vals_low = _mm512_unpacklo_epi8(b_vals, _mm512_setzero_si512());
                __m512i b_vals_high = _mm512_unpackhi_epi8(b_vals, _mm512_setzero_si512());

                __m512i mul_low = _mm512_madd_epi16(a_vals_low, b_vals_low);
                __m512i mul_high = _mm512_madd_epi16(a_vals_high, b_vals_high);

                sum = _mm512_add_epi32(sum, mul_low);
                sum = _mm512_add_epi32(sum, mul_high);

                // Accumulate sums for the elements in the block
                if (b < block_size) {
                    __mmask64 mask = (1ULL << b) - 1;
                    sum = _mm512_maskz_add_epi32(mask, sum, _mm512_setzero_si512());  // Mask sum to handle blocks smaller than 64
                }
            }
            // Store the final summed value for each element in result[i][j]
            int res_value = _mm512_reduce_add_epi32(sum);
            result[i][j] = res_value;
        }
    }

    return result;
}



// Specialization for std::vector<bool>
size_t calculateMatrixMemorySize(const std::vector<std::vector<bool>>& matrix) {
    size_t rowSize = matrix.size() * sizeof(std::vector<bool>);
    size_t totalBits = 0;
    for (const auto& row : matrix) {
        totalBits += row.size();
    }
    size_t totalSize = rowSize + (totalBits + 7) / 8; // Bits to bytes
    return totalSize;
}

int main() {
    auto matrix1 = createRadomMatrix<short>(1000, 1000);
    auto matrix2 = createRadomMatrix<short>(1000,1000);

    auto matrix3 = createRadomMatrix<int8_t>(10000, 10000);
    auto matrix4 = createRadomMatrix<int8_t>(10000,10000);

    auto matrix5 = createRadomMatrix<int>(10000, 10000);
    auto matrix6 = createRadomMatrix<int>(10000,10000);

    auto vector3 = std::vector<bool>(50000, 1);

    double time3 = 0;
    for (int i = 0; i < 1; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = multiplyMatrices(matrix5, matrix6);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time3 += duration.count();
    }

    std::cout << "Laufzeit für normal int (1000,1000)x(1000,1000): " << time3 << " ms" << std::endl;


    double time4 = 0;
    for (int i = 0; i < 1; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul = multiplyMatrices(matrix1, matrix2);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time4 += duration.count();
    }

    std::cout << "Laufzeit für normal short (10000,10000)x(10000,10000): " << time4 << " ms" << std::endl;

    double time6 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512(matrix5, matrix6);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time6 += duration.count();
    }

    std::cout << "Laufzeit für avx int (1000,1000)x(1000,1000): " << time6/10 << " ms" << std::endl;


    double time7 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit(matrix3, matrix4);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time7 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 (10000,10000)x(10000,10000): " << time7/10 << " ms" << std::endl;

    double time8 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit_Transposed(matrix3, matrix4);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time8 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 (10000,10000)x(10000,10000): " << time8/10 << " ms" << std::endl;

    double time9 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit_custom_block_size(matrix3, matrix4, 128);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time9 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 with 128er blocks (10000,10000)x(10000,10000): " << time9/10 << " ms" << std::endl;


    double time10 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit_custom_block_size(matrix3, matrix4, 256);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time10 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 with 256er blocks (10000,10000)x(10000,10000): " << time10 /10<< " ms" << std::endl;



    double time11 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit_custom_block_size(matrix3, matrix4, 512);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time11 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 with 512er blocks (10000,10000)x(10000,10000): " << time11 /10<< " ms" << std::endl;


    double time12 = 0;
    for (int i = 0; i < 10; i++) {
        auto start = std::chrono::high_resolution_clock::now();

        // Matrix-Vektor-Multiplikation
        auto mul =  multiplyMatricesAVX512_8bit_custom_block_size(matrix3, matrix4, 1024);

        // Zeitmessung stoppen
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start;
        time12 += duration.count();
    }

    std::cout << "Laufzeit für avx int_8 with 1024er blocks (10000,10000)x(10000,10000): " << time11 /10<< " ms" << std::endl;


    return 0;
}
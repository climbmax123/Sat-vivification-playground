# The concept behind Fast CNF Processing

The goal of FasterCnfProcessing is it to make a Implementation of sat-solving that is less predicative.

Modern Sat Solvers focus a lot on algorithms and little on the strength of their processors. 
Therefore, a lot of the methods are implemented in a way that a lot of branch misses can cache misses are produced.

The goal of this project is to write it as fast as possible. Therefore we will try to find a Implemenation that is 
highly parallelize and furthermore very predictive.

## Remarks To the Concept

I want to implement most of the Operations with matrix operation.
Encoding a Sat formula as a matrix yields the possibility to put it on a GPU. That would enable a significant 
performance increase of nearly possible 4000 Trillion operations per second.
This is a significantly more than an any CPU can do.

Furthermore, we can enhance the performance of the cpu by using the simd properties of the gpu. Here we can use AVX512 
to increase the compute performance of the formula.

## Data types used:

For compute we can use a array of bools but that would be slower that using int8 

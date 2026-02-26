#ifndef BASE_H
#define BASE_H

#define FP_EPS 1e-12

#include <stddef.h>
#include <stdint.h>

typedef uint32_t index_t;
typedef double   real_t;

#define PI 3.14159265358979323846

void* xmalloc(size_t size);

void fillN(real_t* Res, real_t value, index_t n);
void zeroN(real_t* Res, index_t n);
void copyN(real_t* Res, const real_t* v, index_t n);

#ifdef COL_MAJOR
#        define MAT_IDX(row, col, rows, cols) ((col) * (rows) + (row))
#else
#        define MAT_IDX(row, col, rows, cols) ((row) * (cols) + (col))
#endif
#define MAT3IDX(row, col) MAT_IDX((row), (col), 3, 3)
#define MAT4IDX(row, col) MAT_IDX((row), (col), 4, 4)

#endif

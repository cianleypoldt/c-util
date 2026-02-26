#include "base.h"

#include <stdio.h>

void vec_print(const real_t* v, index_t n) {
        printf("{ ");
        for (index_t i = 0; i < n - 1; i++) {
                printf("%f, ", (float) v[i]);
        }
        printf("%f }\n", (float) v[n - 1]);
}

void vec_print_named(const real_t* v, index_t n, const char* name) {
        printf("vec%i \"%s\":\n", (int) n, name);
        vec_print(v, n);
}

void mat_print(const real_t* M, index_t rows, index_t cols) {
        for (index_t row = 0; row < rows; row++) {
                printf("{ ");
                for (index_t col = 0; col < cols - 1; col++) {
                        printf("%f, ", (float) M[MAT_IDX(row, col, rows, cols)]);
                }
                printf("%f }\n", (float) M[MAT_IDX(row, cols - 1, rows, cols)]);
        }
}

void mat_print_named(const real_t* M, index_t rows, index_t cols, const char* name) {
        printf("mat%ix%i \"%s\":\n", (int) rows, (int) cols, name);
        mat_print(M, rows, cols);
}

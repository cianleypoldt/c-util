#ifndef DEBUG_H
#define DEBUG_H

#include "base.h"

void vec_print(const real_t* v, index_t n);
void vec_print_named(const real_t* v, index_t n, const char* name);
void mat_print(const real_t* M, index_t rows, index_t cols);
void mat_print_named(const real_t* M, index_t rows, index_t cols, const char* name);

#endif

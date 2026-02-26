#ifndef ARRAY_H
#define ARRAY_H

#include <stddef.h>

#define ARRAY_BASE_COUNT 1
#define ARRAY_RESIZE_FACTOR 2

typedef struct {
        void* data;
        size_t count;
        size_t capacity;
        size_t element_size;
} DynamicArray;

DynamicArray DA_create(size_t element_size);
void DA_delete(DynamicArray* array);
void* DA_at(const DynamicArray* array, size_t index);
void DA_swap_elements(DynamicArray* array, size_t index_a, size_t index_b);
void DA_resize(DynamicArray* array, size_t capacity);
void DA_append(DynamicArray* array, const void* data);
void DA_remove_at(DynamicArray* array, size_t index);
void DA_remove_swap_at(DynamicArray* array, size_t index);

typedef struct {
        void* data;
        unsigned char* occup;
        size_t max_index;
        size_t capacity;
        size_t element_size;
} FreeList;

FreeList FL_create(size_t element_size);
void FL_resize(FreeList* array, size_t capacity);
size_t FL_add(FreeList* array, const void* data);
void* FL_at(const FreeList* array, size_t index);
void FL_delete(FreeList* array);
void FL_remove_at(FreeList* array, size_t index);

typedef size_t SM_id;

typedef struct {
        FreeList index_map;
        DynamicArray data;
} SlotMap;

SlotMap SM_create(size_t element_size);
void SM_delete(SlotMap* array);
SM_id SM_get_index(const SlotMap* sm, SM_id id);
void* SM_at_id(const SlotMap* array, SM_id id);
void SM_swap_elements(SlotMap* sm, SM_id id_a, SM_id id_b);
void SM_resize(SlotMap* sm, size_t capacity);
SM_id SM_add(SlotMap* sm, const void* data);
void SM_remove_at_id(SlotMap* sm, SM_id id);
void SM_remove_swap_at_id(SlotMap* sm, SM_id id);

#endif

#ifndef IOT_CAR_ARRAY
#define IOT_CAR_ARRAY 1

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint32_t const MAX_SIZE = 0xFFFFFFFF;

/*
  动态数组。
  实现类似于std::vector的功能，用来存储元素。
  元素内部全部采用空指针以充当简化泛型。
  还没有检验。
*/
typedef struct {
    uint32_t size;
    uint32_t current_max;
    void* p;
    size_t elem_size;
} array;

void create_array(array* arr, uint32_t size, size_t elem_size) {
    arr->p = malloc(size * elem_size);
    arr->size = 0;
    arr->current_max = size;
    arr->elem_size = elem_size;
}

uint32_t arr_len(const array* arr) {
    return arr->size;
}

uint8_t _check_size(array* arr) {
    if (arr->size >= arr->current_max) {
        if (arr->current_max >= MAX_SIZE / 2) {
            arr->current_max = MAX_SIZE;
        }
        else if (arr->current_max == MAX_SIZE) {
            return 0;
        }
        else {
            arr->current_max *= 2;
        }
        void* new_p = realloc(arr->p, arr->current_max * arr->elem_size);
        if (!new_p) {
            return 0;
        }
        arr->p = new_p;
    }
    return 1;
}

void push_back(array* arr, void* elem) {
    if (!_check_size(arr)) {
        // TODO: handle error
        return;
    }
    memcpy((char*)arr->p + arr->size * arr->elem_size, elem, arr->elem_size);
    arr->size += 1;
}

void* back(const array* arr) {
    if (arr->size == 0) {
        return NULL;
    }
    return (char*)arr->p + (arr->size - 1) * arr->elem_size;
}

void delete_arr(array* arr) {
    free(arr->p);
    arr->p = NULL;
    arr->size = 0;
    arr->current_max = 0;
}

void delete_elem(array* arr, uint32_t position) {
    if (position < arr->size) {
        char* base = (char*)arr->p;
        memmove(base + position * arr->elem_size,
            base + (position + 1) * arr->elem_size,
            (arr->size - position - 1) * arr->elem_size);
        arr->size -= 1;
    }
}

#endif

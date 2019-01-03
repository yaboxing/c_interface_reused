#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../assert/assert.h"
#include "../mem/mem.h"
#include "array.h"

#define T Array_T

void ArrayRep_init(T array, int length, int size, void* ary)
{
    assert(array);
    assert((ary && length>0) || (length==0 && ary==NULL));
    assert(size > 0);

    array->length = length;
    array->size = size;
    array->array = length > 0 ? ary : NULL;
}

T Array_new(int length, int size)
{
    T array;

    NEW(array);
    if (length > 0) {
        ArrayRep_init(array, length, size, CALLOC(length, size));
    } else {
        ArrayRep_init(array, length, size, NULL);
    }

    return array;
}

void Array_free(T* array)
{
    assert(array && * array);
    FREE((*array)->array);
    FREE(*array);
}

int Array_length(T array)
{
    assert(array);
    return array->length;
}

int Array_size(T array)
{
    assert(array);
    return array->size;
}

void* Array_get(T array, int i)
{
    assert(array);
    assert(i >= 0 && i < array->length);

    return array->array + i * array->size;
}

void* Array_put(T array, int i, void* elem)
{
    assert(array);
    assert(i >= 0 && i < array->length);
    assert(elem);

    memcpy(array->array + i * array->size,
           elem,
           array->size);

    return elem;
}

void Array_resize(T array, int length)
{
    assert(array);
    assert(length >= 0);

    if (array->length == 0) {
        array->array = ALLOC(length * array->size);
    } else if (length > 0) {
        RESIZE(array->array, length * array->size);
    } else {
        FREE(array->array);
    }

    array->length = length;
}

T Array_copy(T array, int length)
{
    T copy;

    assert(array);
    assert(length >= 0);
    copy = Array_new(length, array->size);
    if (copy->length >= array->length && array->length > 0) {
        memcpy(copy->array, array->array, array->length);

    } else if (array->length > copy->length && copy->length > 0) {
        memcpy(copy->array, array->array, copy->length);
    }

    return copy;
}


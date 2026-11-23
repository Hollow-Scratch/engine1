#ifndef HOLLOWENGINE_VECTOR_HEADER
#define HOLLOWENGINE_VECTOR_HEADER

#include <stddef.h>
#include <stdint.h>

typedef struct vector {
	int32_t *data;
	size_t length;
	size_t capacity;
} vector;

void vector_append(vector *vec, size_t value);

#endif

#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct vector {
	void *data;
	size_t length;
	size_t capacity;
	size_t element_size;
} vector;

#define VECTOR_INIT(type) (vector){ .element_size = sizeof(type) }
#define VECTOR_GET(vec, type, index) (((type *)(vec).data)[index])
#define VECTOR_AUTO __attribute__((cleanup(vector_free))) vector

void vector_append(vector *vec, const void *value);
bool vector_pop(vector *vec, void *out_value);
void vector_resize(vector *vec, size_t new_length);
void vector_free(vector *vec);

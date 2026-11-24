#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct Vector {
	void  *data;
	size_t length;
	size_t capacity;
	size_t element_size;
} Vector;

#define VECTOR_INIT(type)                                                                          \
	(Vector) {                                                                                     \
		.element_size = sizeof(type)                                                               \
	}
#define VECTOR_GET(vec, type, index) (((type *)(vec).data)[index])
#define VECTOR_AUTO __attribute__((cleanup(VectorFree))) Vector

void VectorAppend(Vector *vec, const void *value);
bool VectorPop(Vector *vec, void *out_value);
void VectorResize(Vector *vec, size_t new_length);
void VectorFree(Vector *vec);

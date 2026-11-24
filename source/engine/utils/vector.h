#pragma once

#include <stdbool.h>
#include <stddef.h>

typedef struct Vector {
	void *data;
	size_t length;
	size_t capacity;
	size_t elementSize;
} Vector;

#define VECTOR_INIT(type)                                                      \
	(Vector) { .elementSize = sizeof(type) }
#define VECTOR_GET(vec, type, index) (((type *)(vec).data)[index])
#define VECTOR_AUTO __attribute__((cleanup(vectorFree))) Vector

void vectorAppend(Vector *vec, const void *value);
bool vectorPop(Vector *vec, void *outValue);
void vectorResize(Vector *vec, size_t newLength);
void vectorFree(Vector *vec);

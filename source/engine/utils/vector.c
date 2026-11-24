#include "vector.h"

#include <stdlib.h>
#include <string.h>

void vectorAppend(Vector *vec, const void *value) {
	if (vec->length >= vec->capacity) {
		vec->capacity = (vec->capacity == 0) ? 2 : vec->capacity * 2;
		vec->data	  = realloc(vec->data, vec->capacity * vec->elementSize);
	}
	void *dest = (char *)vec->data + (vec->length * vec->elementSize);
	memcpy(dest, value, vec->elementSize);
	vec->length++;
}

bool vectorPop(Vector *vec, void *outValue) {
	if (vec->length == 0) {
		return false;
	}
	vec->length--;
	if (outValue != NULL) {
		void *src = (char *)vec->data + (vec->length * vec->elementSize);
		memcpy(outValue, src, vec->elementSize);
	}
	return true;
}

void vectorResize(Vector *vec, size_t newLength) {
	if (newLength > vec->capacity) {
		vec->capacity = newLength * 2;
		vec->data	  = realloc(vec->data, vec->capacity * vec->elementSize);
	}
	vec->length = newLength;
}

void vectorFree(Vector *vec) {
	free(vec->data);
	vec->data	  = NULL;
	vec->length	  = 0;
	vec->capacity = 0;
}

#include "vector.h"

#include <stdlib.h>
#include <string.h>

void VectorAppend(Vector *vec, const void *value) {
	if (vec->length >= vec->capacity) {
		vec->capacity = (vec->capacity == 0) ? 2 : vec->capacity * 2;
		vec->data	  = realloc(vec->data, vec->capacity * vec->element_size);
	}
	void *dest = (char *)vec->data + (vec->length * vec->element_size);
	memcpy(dest, value, vec->element_size);
	vec->length++;
}

bool VectorPop(Vector *vec, void *out_value) {
	if (vec->length == 0) {
		return false;
	}
	vec->length--;
	if (out_value != NULL) {
		void *src = (char *)vec->data + (vec->length * vec->element_size);
		memcpy(out_value, src, vec->element_size);
	}
	return true;
}

void VectorResize(Vector *vec, size_t new_length) {
	if (new_length > vec->capacity) {
		vec->capacity = new_length * 2;
		vec->data	  = realloc(vec->data, vec->capacity * vec->element_size);
	}
	vec->length = new_length;
}

void VectorFree(Vector *vec) {
	free(vec->data);
	vec->data	  = NULL;
	vec->length	  = 0;
	vec->capacity = 0;
}

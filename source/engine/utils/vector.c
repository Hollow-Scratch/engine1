#include "vector.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void vector_append(vector *vec, size_t value) {
	if (vec->length >= vec->capacity) {
		if (vec->capacity == 0) {
			vec->capacity = 2;
		} else {
			vec->capacity *= 2;
		}
		vec->data = realloc(vec->data, vec->capacity * sizeof(*vec->data));
	}
	vec->data[vec->length++] = value;
}

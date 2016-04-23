#include "bytearray.h"
#include "panic.h"

#include <stdlib.h>
#include <strings.h>
#include <assert.h>
#include <string.h>

static inline int max(int a, int b)
{
    return a > b ? a : b;
}

struct byte_array * byte_array_new(int capacity)
{
    struct byte_array *b;

    b = malloc(sizeof *b);

    b->capacity = capacity;
    b->size = 0;

    b->data = calloc(capacity, sizeof (char));
    b->end = b->data;

    bzero(b->data, capacity * sizeof (char));

    return b;
}

void byte_array_free(struct byte_array *b)
{
    free(b->data);
    free(b);
}

void byte_array_append(struct byte_array *b, const char *data, size_t size)
{
    size_t new_size;

    if (size <= 0)
        return;

    new_size = b->size + size;

    if (new_size > b->capacity)
        byte_array_resize(b, max(new_size + 1, b->capacity << 1));

    memcpy(b->end, data, size);
    b->end += size;
    b->size +=size;
}

void byte_array_resize(struct byte_array *b, size_t capacity)
{
    char *ptr;

    size_t old_capacity;

    assert(capacity > 0);

    ptr = (char *) realloc(b->data, capacity);

    if (ptr == NULL)
        panic("Not enough memory");

    b->data = ptr;
    b->end = b->data + b->size;
    old_capacity = b->capacity;
    b->capacity = capacity;

    if (b->size > capacity) {
        b->size = capacity;
    } else if (b->capacity > old_capacity) {
        bzero((b->data + old_capacity), capacity - old_capacity);
    }
}

char * byte_array_data(struct byte_array *b)
{
    return b->data;
}

char * byte_array_detach(struct byte_array *b)
{
    char *data;

    data = b->data;

    free(b);

    return data;
}

size_t byte_array_size(struct byte_array *b)
{
    return b->size;
}

size_t byte_array_capacity(struct byte_array *b)
{
    return b->capacity;
}


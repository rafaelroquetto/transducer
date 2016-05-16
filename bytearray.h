#ifndef BYTERRAY_H
#define BYTERRAY_H

#include <unistd.h>

struct byte_array
{
    char *data;
    char *end;

    size_t capacity;
    size_t size;
};

struct byte_array *byte_array_new(int capacity);

void byte_array_free(struct byte_array *b);
void byte_array_append(struct byte_array *b, const char *data, size_t size);
void byte_array_append_char(struct byte_array *b, char ch);
void byte_array_push(struct byte_array *b, char ch);
void byte_array_resize(struct byte_array *b, size_t size);

char byte_array_pop(struct byte_array *b);

char * byte_array_data(struct byte_array *b);
char * byte_array_detach(struct byte_array *b);

size_t byte_array_size(struct byte_array *b);
size_t byte_array_capacity(struct byte_array *b);

#endif /* BYTERRAY_H */

#include "input.h"
#include "bytearray.h"

#include <strings.h>
#include <unistd.h>
#include <stdio.h>

char * read_input(void)
{
    static char buf[256];
    struct byte_array *b;
    int nbytes;

    bzero(buf, sizeof buf);

    b = byte_array_new(256);

    while (nbytes = read(STDIN_FILENO, buf, sizeof buf))
        byte_array_append(b, buf, nbytes);

    return byte_array_detach(b);
}


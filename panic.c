#include "panic.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void panic(const char *fmt, ...)
{
    va_list ap;

    fflush(stdout);
    fflush(stderr);

    fprintf(stderr, "FATAL: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    fputc('\n', stderr);

    exit(1);
}


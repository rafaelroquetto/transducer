#include "readline.h"

#include <strings.h>
#include <unistd.h>

char * read_line(void)
{
    static char buf[256];
    int nbytes;

    bzero(buf, sizeof buf);

    do {
        nbytes = read(STDIN_FILENO, buf, sizeof buf);
    } while (nbytes == -1);

    buf[nbytes - 1] = '\0';

    return buf;
}


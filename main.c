#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "statemachine.h"
#include "readline.h"

enum {
    MAX_TOKEN_SIZE = 32,
    MAX_TOKENS = 100
};

static int temp_n;
static char temp_s[MAX_TOKEN_SIZE];
static char id[MAX_TOKENS][MAX_TOKEN_SIZE];
static char *ptr;

static int find_token(const char *token)
{
    int i;

    for (i = 0; i < MAX_TOKENS; ++i) {
        if (strcmp(token, id[i]) == 0)
            return i;
    }

    return -1;
}

static int append_token(const char *token)
{
    static int pos = 0;

    strncpy(id[pos], token, MAX_TOKEN_SIZE);

    return pos++;
}

static void sigma1(char ch)
{
    ptr = temp_s;

    *ptr++ = ch;
}

static void sigma2(char ch)
{
    *ptr++ = ch;
}

static void sigma3(char ch)
{
    int pos;

    *ptr++ = 0;

    pos = find_token(temp_s);

    if (pos == -1) {
        pos = append_token(temp_s);
    }

    printf("V(%d) ", pos);
}

static void sigma4(char ch)
{
    temp_n = ch - '0';
}

static void sigma5(char ch)
{
    temp_n *= 10;
    temp_n += ch - '0';
}

static void sigma6(char ch)
{
    printf("N(%d) ", temp_n);
}

int main(int argc, char *argv[])
{
    struct transition *t;
    struct state_machine *sm = sm_new();

    char *input;

    memset(id, 0, sizeof id);

    int rc;
    int fstates[] = { 0 };

    sm_set_nstates(sm, 3);
    sm_set_fstates(sm, fstates, sizeof (fstates) / sizeof(int));

    /* eo -> e0 */
    add_transition(sm, 0, 0, ' ', NULL);

    /* e0 -> e1 */
    add_range_transition(sm, 0, 1, 'A', 'Z', sigma1);

    /* e1 -> e1 */
    add_range_transition(sm, 1, 1, 'A', 'Z', sigma2);
    add_range_transition(sm, 1, 1, '0', '9', sigma2);

    /* e1 -> e0 */
    add_epsilon_transition(sm, 1, 0, sigma3);

    /* e0 -> e2 */
    add_range_transition(sm, 0, 2, '0', '9', sigma4);

    /* e2 -> e2 */
    add_range_transition(sm, 2, 2, '0', '9', sigma5);

    /* e2 -> e0 */
    add_epsilon_transition(sm, 2, 0, sigma6);

    input = read_line();

    sm_set_input_buffer(sm, input);

    rc = sm_exec(sm);

    sm_free(sm);

    putchar('\n');

    return rc;
}

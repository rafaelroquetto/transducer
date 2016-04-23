#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "statemachine.h"
#include "input.h"

enum {
    MAX_TOKEN_SIZE = 32,
    MAX_TOKENS = 100
};

enum States { e0, e1, e2, e3, e4, e5, e6, N_STATES };

static int temp_n;
static char temp_s[MAX_TOKEN_SIZE];
static char id[MAX_TOKENS][MAX_TOKEN_SIZE];
static char *ptr;

static const char *reserved_words[] = {
    "IF", "THEN", "ELSE", "GOTO", "LET",
    "PRINT", "OF", "READ", "END", NULL
};


static int find_keyword(const char *word)
{
    int i;

    for (i = 0; *(reserved_words + i); ++i) {
        if (strcmp(word, *(reserved_words +i)) == 0)
            return i;
    }

    return -1;
}

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

    if ((pos = find_keyword(temp_s)) >= 0) {
        printf("P(%d) ", pos);
        return;
    }

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

static void sigma7(char ch)
{
    printf(":= ");
}

static void sigma8(char ch)
{
    printf(": ");
}

static void sigma9(char ch)
{
    printf("%c ", ch);
}

int main(int argc, char *argv[])
{
    struct transition *t;
    struct state_machine *sm = sm_new();

    char *input;

    memset(id, 0, sizeof id);

    int rc;
    int fstates[] = { 0 };

    sm_set_nstates(sm, N_STATES);
    sm_set_fstates(sm, fstates, sizeof (fstates) / sizeof(int));

    /* eo -> e0 */
    add_transition(sm, e0, e0, ' ', NULL);
    add_transition(sm, e0, e0, '\n', NULL);

    /* e0 -> e1 */
    add_range_transition(sm, e0, e1, 'A', 'Z', sigma1);

    /* e1 -> e1 */
    add_range_transition(sm, e1, e1, 'A', 'Z', sigma2);
    add_range_transition(sm, e1, e1, '0', '9', sigma2);

    /* e1 -> e0 */
    add_epsilon_transition(sm, e1, e0, sigma3);

    /* e0 -> e2 */
    add_range_transition(sm, e0, e2, '0', '9', sigma4);

    /* e2 -> e2 */
    add_range_transition(sm, e2, e2, '0', '9', sigma5);

    /* e2 -> e0 */
    add_epsilon_transition(sm, e2, e0, sigma6);

    /* e0 -> e3 */
    add_transition(sm, e0, e3, ':', NULL);

    /* e3 -> e4 */
    add_transition(sm, e3, e4, '=', NULL);

    /* e3 -> e0 */
    add_epsilon_transition(sm, e3, e0, sigma8);

    /* e4 -> e0 */
    add_epsilon_transition(sm, e4, e0, sigma7);

    /* e0 -> e5 */
    add_transition(sm, e0, e5, '+', sigma9);
    add_transition(sm, e0, e5, '-', sigma9);
    add_transition(sm, e0, e5, '*', sigma9);
    add_transition(sm, e0, e5, '/', sigma9);
    add_transition(sm, e0, e5, '(', sigma9);
    add_transition(sm, e0, e5, ')', sigma9);
    add_transition(sm, e0, e5, '>', sigma9);
    add_transition(sm, e0, e5, '<', sigma9);
    add_transition(sm, e0, e5, '=', sigma9);

    /* e5 -> e0 */
    add_epsilon_transition(sm, e5, e0, NULL);

    /* e0 -> e6*/
    add_transition(sm, e0, e6, '%', NULL);

    /* e6 -> e6 */
    add_transition(sm, e6, e6, '%', NULL);
    add_transition(sm, e6, e6, '+', NULL);
    add_transition(sm, e6, e6, '-', NULL);
    add_transition(sm, e6, e6, '*', NULL);
    add_transition(sm, e6, e6, '/', NULL);
    add_transition(sm, e6, e6, '(', NULL);
    add_transition(sm, e6, e6, ')', NULL);
    add_transition(sm, e6, e6, '>', NULL);
    add_transition(sm, e6, e6, '<', NULL);
    add_transition(sm, e6, e6, '=', NULL);
    add_transition(sm, e6, e6, ':', NULL);
    add_transition(sm, e6, e6, ' ', NULL);
    add_range_transition(sm, e6, e6, '0', '9', NULL);
    add_range_transition(sm, e6, e6, 'A', 'Z', NULL);

    /* e6 -> e0 */
    add_transition(sm, e6, e0, '\n', NULL);
    add_epsilon_transition(sm, e6, e0, NULL);

    if (isatty(STDIN_FILENO))
        printf("Running on interactive mode. Press (CTRL+D) to end\n\n");

    input = read_input();

    sm_set_input_buffer(sm, input);

    rc = sm_exec(sm);

    sm_free(sm);

    free(input);

    putchar('\n');

    return rc;
}

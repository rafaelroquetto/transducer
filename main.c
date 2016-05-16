#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "statemachine.h"
#include "input.h"
#include "panic.h"
#include "bytearray.h"

enum {
    MAX_TOKEN_SIZE = 32,
    MAX_TOKENS = 100
};

enum States { e0, e1, e2, e3, e4, e5, e6, N_STATES };

enum Types {
    NUMBER = 1,
    SYMBOL,
    KEYWORD,
    VARIABLE,
    LPARENS,
    RPARENS
};

static int temp_n;
static char temp_s[MAX_TOKEN_SIZE];
static char id[MAX_TOKENS][MAX_TOKEN_SIZE];
static char *ptr;

static struct byte_array *lexer_buffer;

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

typedef void (*lexer_cb)(int type, void *data);

struct lexer
{
    struct state_machine *sm;
    lexer_cb callback;
};

struct lexer * lx_new(struct state_machine *sm)
{
    struct lexer *l;

    l = malloc(sizeof *l);
    l->sm = sm;
    l->callback = NULL;

    return l;
}

static void lx_free(struct lexer *l)
{
    sm_free(l->sm);
    free(l);
}

static void lx_set_input(struct lexer *l, const char *buf)
{
    sm_set_input_buffer(l->sm, buf);
}

static void lx_set_callback(struct lexer *l, lexer_cb callback)
{
    l->callback = callback;
}

static int lx_exec(struct lexer *l)
{
    int rc;
    rc = sm_exec(l->sm);

    if (rc == -1)
        panic("Invalid token encountered");

    return 0;
}

static void sigma1(char ch, void *data)
{
    ptr = temp_s;

    *ptr++ = ch;
}

static void sigma2(char ch, void *data)
{
    *ptr++ = ch;
}

static void sigma3(char ch, void *data)
{
    struct lexer *l;
    int pos;

    l = (struct lexer *) data;

    *ptr++ = 0;

    if ((pos = find_keyword(temp_s)) >= 0) {
        if (l->callback)
            l->callback(KEYWORD, (void *) reserved_words[pos]);
        return;
    }

    pos = find_token(temp_s);

    if (pos == -1) {
        pos = append_token(temp_s);
    }

    if (l->callback)
        l->callback(VARIABLE, id[pos]);
}

static void sigma4(char ch, void *data)
{
    temp_n = ch - '0';
}

static void sigma5(char ch, void *data)
{
    temp_n *= 10;
    temp_n += ch - '0';
}

static void sigma6(char ch, void *data)
{
    struct lexer *l;

    l = (struct lexer *) data;

    if (l->callback)
        l->callback(NUMBER, &temp_n);
}

static void sigma7(char ch, void *data)
{
    printf(":= ");
}

static void sigma8(char ch, void *data)
{
    printf(": ");
}

static void sigma9(char ch, void *data)
{
    struct lexer *l;
    l = (struct lexer *) data;

    if (l->callback)
        l->callback(SYMBOL, &ch);
}

static void sigma10(char ch, void *data)
{
    struct lexer *l;
    l = (struct lexer *) data;

    if (l->callback)
        l->callback(LPARENS, &ch);
}

static void sigma11(char ch, void *data)
{
    struct lexer *l;
    l = (struct lexer *) data;

    if (l->callback)
        l->callback(RPARENS, &ch);
}


static struct lexer * create_lexer(void)
{
    struct transition *t;
    struct state_machine *sm = sm_new();
    struct lexer *lexer = lx_new(sm);

    memset(id, 0, sizeof id);

    int fstates[] = { 0 };

    sm_set_nstates(sm, N_STATES);
    sm_set_istate(sm, e0);
    sm_set_fstates(sm, fstates, sizeof (fstates) / sizeof(int));

    /* eo -> e0 */
    add_transition(sm, e0, e0, ' ', NULL, NULL);
    add_transition(sm, e0, e0, '\n', NULL, NULL);

    /* e0 -> e1 */
    add_range_transition(sm, e0, e1, 'A', 'Z', sigma1, NULL);

    /* e1 -> e1 */
    add_range_transition(sm, e1, e1, 'A', 'Z', sigma2, NULL);
    add_range_transition(sm, e1, e1, '0', '9', sigma2, NULL);

    /* e1 -> e0 */
    add_epsilon_transition(sm, e1, e0, sigma3, (void *) lexer);

    /* e0 -> e2 */
    add_range_transition(sm, e0, e2, '0', '9', sigma4, NULL);

    /* e2 -> e2 */
    add_range_transition(sm, e2, e2, '0', '9', sigma5, NULL);

    /* e2 -> e0 */
    add_epsilon_transition(sm, e2, e0, sigma6, (void *) lexer);

    /* e0 -> e3 */
    add_transition(sm, e0, e3, ':', NULL, NULL);

    /* e3 -> e4 */
    add_transition(sm, e3, e4, '=', NULL, NULL);

    /* e3 -> e0 */
    add_epsilon_transition(sm, e3, e0, sigma8, NULL);

    /* e4 -> e0 */
    add_epsilon_transition(sm, e4, e0, sigma7, NULL);

    /* e0 -> e5 */
    add_transition(sm, e0, e5, '+', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '-', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '*', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '/', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '(', sigma10, (void *) lexer);
    add_transition(sm, e0, e5, ')', sigma11, (void *) lexer);
    add_transition(sm, e0, e5, '>', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '<', sigma9, (void *) lexer);
    add_transition(sm, e0, e5, '=', sigma9, (void *) lexer);

    /* e5 -> e0 */
    add_epsilon_transition(sm, e5, e0, NULL, NULL);

    /* e0 -> e6*/
    add_transition(sm, e0, e6, '%', NULL, NULL);

    /* e6 -> e6 */
    add_transition(sm, e6, e6, '%', NULL, NULL);
    add_transition(sm, e6, e6, '+', NULL, NULL);
    add_transition(sm, e6, e6, '-', NULL, NULL);
    add_transition(sm, e6, e6, '*', NULL, NULL);
    add_transition(sm, e6, e6, '/', NULL, NULL);
    add_transition(sm, e6, e6, '(', NULL, NULL);
    add_transition(sm, e6, e6, ')', NULL, NULL);
    add_transition(sm, e6, e6, '>', NULL, NULL);
    add_transition(sm, e6, e6, '<', NULL, NULL);
    add_transition(sm, e6, e6, '=', NULL, NULL);
    add_transition(sm, e6, e6, ':', NULL, NULL);
    add_transition(sm, e6, e6, ' ', NULL, NULL);
    add_range_transition(sm, e6, e6, '0', '9', NULL, NULL);
    add_range_transition(sm, e6, e6, 'A', 'Z', NULL, NULL);

    /* e6 -> e0 */
    add_transition(sm, e6, e0, '\n', NULL, NULL);
    add_epsilon_transition(sm, e6, e0, NULL, NULL);

    return lexer;
}


enum ParserStates { A, B, C, D, N_PSTATES };

static void run_sub_machine(char ch, void *data)
{
    struct state_machine *parser = (struct state_machine *) data;

    sm_exec(parser);
}

static struct state_machine * create_parser(void)
{
    struct state_machine *sm = sm_new();

    sm_set_nstates(sm, N_PSTATES);

    int fstates[] = { D };

    sm_set_istate(sm, A);
    sm_set_fstates(sm, fstates, sizeof (fstates) / sizeof(int));

    /* A -> D */
    add_transition(sm, A, D, NUMBER, NULL, NULL);

    /* A -> B */
    add_transition(sm, A, B, LPARENS, NULL, NULL); 

    /* B -> C */
    add_epsilon_transition(sm, B, C, run_sub_machine, (void*) sm);

    /* C -> D */
    add_transition(sm, C, D, RPARENS, NULL, NULL);

    /* D -> A */
    add_transition(sm, D, A, SYMBOL, NULL, NULL);
}

static void parse(const char *input)
{
    int rc;

    struct state_machine *parser;

    parser = create_parser();

    sm_set_input_buffer(parser, input);

    rc = sm_exec(parser);

    if (rc == -1)
        panic("Syntax error!");

    printf("Syntax OK!\n");

    sm_free(parser);

}

static void lexer_callback(int token_type, void *token_data)
{
    switch (token_type) {
    case NUMBER:
        printf("N(%d) ", *((int *) token_data));
        break;
    case SYMBOL:
        printf("S(%c) ", *((char *) token_data));
        break;
    case KEYWORD:
        printf("K(%s) ", (char *) token_data);
        break;
    case VARIABLE:
        printf("V(%s) ", (char *) token_data);
        break;
    case LPARENS:
        printf("LPARENS ");
        break;
    case RPARENS:
        printf("RPARENS ");
        break;
    }

    byte_array_append_char(lexer_buffer, (char) token_type);
}

int main(int argc, char *argv[])
{
    char *input;
    int rc;

    struct lexer *lexer;
    struct state_machine *parser;

    if (isatty(STDIN_FILENO))
        printf("Running on interactive mode. Press (CTRL+D) to end\n\n");

    /* read input */
    input = read_input();

    /* buffer with final lexer results */
    lexer_buffer = byte_array_new(16);

    lexer = create_lexer();

    lx_set_input(lexer, input);
    lx_set_callback(lexer, lexer_callback);

    /* lex it! */
    rc = lx_exec(lexer);

    lx_free(lexer);

    free(input);


    putchar('\n');

    /* so far so good, now verify the syntax */

    parse(byte_array_data(lexer_buffer));

    byte_array_free(lexer_buffer);

    return rc;
}

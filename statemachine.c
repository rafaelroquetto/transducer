#include "statemachine.h"
#include "panic.h"

#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

static char EPSILON_TRANSITION = 0;

enum { DEBUG = 0 };

static struct transition * get_transition(struct state_machine *sm, int state, char token)
{
    int i;
    struct transition *transition;

    for (i = 0; i < sm->t_used; ++i) {
        transition = *(sm->transitions + i);

        if (transition->source_state != state)
            continue;

        if (transition->token == token)
            return transition;
    }

    return NULL;
}

void sm_initialize(struct state_machine *sm)
{
    bzero(sm, sizeof (*sm));
    sm->t_capacity = 16;
}

void sm_add_transition(struct state_machine *sm, struct transition *t)
{
    if (sm->transitions == NULL) {
        sm->transitions = (struct transition **) calloc(sm->t_capacity, sizeof (t));
        bzero(sm->transitions, sm->t_capacity * sizeof (t));
    }

    if (sm->t_used == (sm->t_capacity - 2)) {
        sm->t_capacity <<= 1;

        struct transition **ptr = (struct transition **)
            realloc(sm->transitions, sm->t_capacity * sizeof (t));

        if (ptr == NULL)
            panic("Not enough memory.");

        sm->transitions = ptr;
    }

    sm->transitions[sm->t_used] = t;
    sm->t_used++;
}

void sm_free_transitions(struct state_machine *sm)
{
    int i;

    for (i = 0; i < sm->t_used; ++i)
        free(*(sm->transitions + i));

    free(sm->transitions);
    free(sm->input);

    sm->t_used = 0;
    sm->t_capacity = 16;
}

void sm_free(struct state_machine *sm)
{
    sm_free_transitions(sm);
    free(sm->fstates);
    free(sm);
}


struct state_machine * sm_new(void)
{
    struct state_machine *sm;

    sm = malloc(sizeof *sm);

    sm_initialize(sm);

    return sm;
}

void sm_set_nstates(struct state_machine *sm, int n)
{
    sm->nstates = n;
}

void sm_set_istate(struct state_machine *sm, int i)
{
    assert(i < sm->nstates);

    sm->istate = i;
}

void sm_set_fstates(struct state_machine *sm, int *fstates, int count)
{
    int i;
    size_t alloc_size = count * sizeof(int *);

    for (i = 0; i < count; ++i) {
        if (fstates[i] >= sm->nstates) {
            printf("Final state %d is out of range, keeping current final states\n", fstates[i]);
            return;
        }
    }

    if (sm->fstates != NULL)
        free(sm->fstates);

    sm->fstates = malloc(alloc_size);
    sm->fs_count = count;
    memcpy(sm->fstates, fstates, alloc_size);
}

struct transition * sm_transition_new(char token, int source_state, int target_state)
{
    struct transition *t;

    t = (struct transition *) malloc(sizeof *t);
    t->token = token;
    t->source_state = source_state;
    t->target_state = target_state;
    t->callback = NULL;

    return t;
}

struct transition * sm_epsilon_transition_new(int source_state, int target_state)
{
    return sm_transition_new(EPSILON_TRANSITION, source_state, target_state);
}

void sm_transition_set_cb(struct transition *t, transition_callback cb)
{
    t->callback = cb;
}

void sm_set_input_buffer(struct state_machine *sm, const char *buffer)
{
    if (sm->input != NULL)
        free(sm->input);

    sm->input = strdup(buffer);
}

inline int sm_has_state(struct state_machine *sm, int state)
{
    return (state < sm->nstates);
}

int sm_is_final_state(struct state_machine *sm, int state)
{
    int i;

    for (i = 0; i < sm->fs_count; ++i) {
        if (*(sm->fstates + i) == state)
            return 1;
    }

    return 0;
}

static struct transition * find_transition(struct state_machine *sm,
        int state, char token)
{
    struct transition *t;

    t = get_transition(sm, state, token);

    if (t != NULL)
        return t;

    return get_transition(sm, state, EPSILON_TRANSITION);
}

int sm_exec(struct state_machine *sm)
{
    char *ptr;
    char ch;
    int current_state;
    struct transition *t;

    ptr = sm->input;

    current_state = sm->istate;

    while (*ptr) {

        if (DEBUG)
            printf("current state: %d; input = '%c'\n", current_state, *ptr);

        t = find_transition(sm, current_state, *ptr);

        if (t == NULL) {
            printf("Invalid input\n");
            return -1;
        }

        if (t->callback)
            t->callback(*ptr);

        current_state = t->target_state;

        if (t->token != EPSILON_TRANSITION)
            ++ptr;
    }

    /* input is fully consumed, check for a final
     * epsilon transition
     */
    t = get_transition(sm, current_state, EPSILON_TRANSITION);

    if (t != NULL) {
        if (t->callback)
            t->callback((char) 0);

        current_state = t->target_state;
    }

    if (DEBUG)
        printf("current state: %d;\n", current_state);

    if (sm_is_final_state(sm, current_state)) {
        printf("Input accepted\n");
        return 0;
    }

    printf("Invalid input\n");

    return -1;
}

void add_transition(struct state_machine *sm, int sstate, int tstate,
        char ch, transition_callback callback)
{
    struct transition *t;

    t = sm_transition_new(ch, sstate, tstate);
    sm_transition_set_cb(t, callback);

    sm_add_transition(sm, t);
}

void add_range_transition(struct state_machine *sm, int sstate, int tstate,
        char start, char end, transition_callback callback)
{
    char ch;

    assert(start < end);

    for (ch = start; ch <= end; ++ch)
        add_transition(sm, sstate, tstate, ch, callback);
}

void add_epsilon_transition(struct state_machine *sm,
        int source_state, int target_state, transition_callback callback)
{
    struct transition *t;

    t = sm_epsilon_transition_new(source_state, target_state);
    sm_transition_set_cb(t, callback);

    sm_add_transition(sm, t);
}


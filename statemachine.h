#ifndef STATEMACHINE_H
#define STATEMACHINE_H

typedef void (*transition_callback)(char, void*);

struct transition
{
    int source_state;
    int target_state;

    char token;

    transition_callback callback;
    void *data;
};

struct state_machine
{
    int nstates;
    int istate;
    int cstate;
    int *fstates;
    int fs_count;

    struct transition **transitions;
    int t_capacity;
    int t_used;

    char *input;
    char *head;
};

void sm_initialize(struct state_machine *sm);
void sm_add_transition(struct state_machine *sm, struct transition *t);
void sm_free_transitions(struct state_machine *sm);
void sm_free(struct state_machine *sm);
void sm_set_nstates(struct state_machine *sm, int n);
void sm_set_istate(struct state_machine *sm, int i);
void sm_set_fstates(struct state_machine *sm, int *fstates, int count);
void sm_transition_set_cb(struct transition *t, transition_callback cb, void *data);
void sm_set_input_buffer(struct state_machine *sm, const char *buffer);


int sm_has_state(struct state_machine *sm, int state);
int sm_is_final_state(struct state_machine *sm, int state);
int sm_exec(struct state_machine *sm);

struct state_machine * sm_new(void);
struct transition * sm_transition_new(char token, int source_state, int target_state);
struct transition * sm_epsilon_transition_new(int source_state, int target_state);

/* convenience functions */

void add_transition(struct state_machine *sm, int sstate, int tstate,
        char ch, transition_callback callback, void *data);
void add_range_transition(struct state_machine *sm, int sstate, int tstate,
        char start, char end, transition_callback callback, void *data);
void add_epsilon_transition(struct state_machine *sm,
        int source_state, int target_state, transition_callback callback, void *data);


#endif /* STATEMACHINE_H */

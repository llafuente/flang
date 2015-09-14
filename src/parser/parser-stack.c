/*
* Copyright 2015 Luis Lafuente <llafuente@noboxout.com>
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "flang.h"

void psr_stack_init(fl_psrstack_t* stack, tk_token_list_t* tokens,
                    fl_psrstate_t* state) {
  stack->current = 0;
  state->look_ahead_idx = 0;
  state->current = 0;
  state->token = &tokens->tokens[0];
  state->next_token = &tokens->tokens[1];
  state->prev_token = 0;
}

void psr_look_ahead(fl_psrstack_t* stack, fl_psrstate_t* state) {
  // printf("- psr_look_ahead [%ld]\n", state->current);

  memcpy(&stack->states[stack->current++], state, sizeof(fl_psrstate_t));
  ++state->look_ahead_idx;
}

void psr_commit(fl_psrstack_t* stack, fl_psrstate_t* state) {
  // printf("- psr_commit [%ld]\n", state->current);
  --stack->current;
  --state->look_ahead_idx;
}

void psr_rollback(fl_psrstack_t* stack, fl_psrstate_t* state) {
  // printf("- psr_rollback [%ld]\n", state->current);

  --stack->current;
  memcpy(state, &stack->states[stack->current], sizeof(fl_psrstate_t));
  // printf("- psr_rollback end [%ld]\n", state->current);
}

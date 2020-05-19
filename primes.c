/* primes.c */

/* generated by GNU MARST -- Algol-to-C Translator, Version 2.7 */
/* Tue May 19 21:38:43 2020 */
/* source file: marst-2.7/examples/primes.alg */
/* object file: primes.c */

#include "algol.h"

extern struct desc main_program_0 /* program */ (void);

extern struct desc outstring_0 /* builtin void procedure */
(     struct arg /* channel: by value integer */,
      struct arg /* str: by name string */
);

extern struct desc outchar_0 /* builtin void procedure */
(     struct arg /* channel: by value integer */,
      struct arg /* str: by name string */,
      struct arg /* int: by value integer */
);

extern struct desc outinteger_0 /* builtin void procedure */
(     struct arg /* channel: by value integer */,
      struct arg /* int: by value integer */
);

struct dsa_main_program_0
{     /* procedure main_program (level 0) declared at line 1 */
      char *proc;
      char *file;
      int line;
      struct dsa *parent;
      struct dsa *vector[0+1];
      /* level of innermost block = 5 */
      struct mem *old_top_0;
      struct mem *new_top_0;
      struct mem *old_top_1;
      struct mem *new_top_1;
      struct mem *old_top_2;
      struct mem *new_top_2;
      jmp_buf jump_2;
      struct mem *old_top_3;
      struct mem *new_top_3;
      struct mem *old_top_4;
      struct mem *new_top_4;
      struct mem *old_top_5;
      struct mem *new_top_5;
      /* procedure block 1 (level 0) beginning at line 1 */
      struct desc retval;
      /* local block 2 (level 1) beginning at line 1 */
      /* FirstFiveHundredPrimes: label
         declared at line 1 and never referenced */
      /* local block 3 (level 2) beginning at line 3 */
      /* j: integer
         declared at line 3 and first referenced at line 5 */
      int j_3;
      /* k: integer
         declared at line 3 and first referenced at line 9 */
      int k_3;
      /* n: integer
         declared at line 3 and first referenced at line 5 */
      int n_3;
      /* q: integer
         declared at line 3 and first referenced at line 10 */
      int q_3;
      /* r: integer
         declared at line 3 and first referenced at line 10 */
      int r_3;
      /* prime: integer array
         declared at line 4 and first referenced at line 5 */
      struct dv *prime_3;
      /* p1: label
         declared at line 5 and never referenced */
      /* p2: label
         declared at line 6 and first referenced at line 12 */
      /* p3: label
         declared at line 7 and never referenced */
      /* p9: label
         declared at line 14 and first referenced at line 7 */
      /* p4: label
         declared at line 8 and first referenced at line 11 */
      /* p5: label
         declared at line 9 and never referenced */
      /* p6: label
         declared at line 10 and first referenced at line 13 */
      /* p7: label
         declared at line 12 and never referenced */
      /* p8: label
         declared at line 13 and never referenced */
      /* teta_r: real
         declared at line 15 and first referenced at line 15 */
      double teta_r_3;
      /* teta_i: integer
         declared at line 15 and first referenced at line 15 */
      int teta_i_3;
      /* local block 4 (level 3) beginning at line 15 */
      /* teta_r: real
         declared at line 17 and first referenced at line 17 */
      double teta_r_4;
      /* teta_i: integer
         declared at line 17 and first referenced at line 17 */
      int teta_i_4;
      /* local block 5 (level 4) beginning at line 17 */
      /* local block 6 (level 5) beginning at line 19 */
      /* p: integer
         declared at line 19 and first referenced at line 20 */
      int p_6;
};

static struct desc _thunk_1(void)
{     /* actual parameter at line 14 */
      struct desc res;
      res.lval = 0;
      res.type = 'i';
      res.u.int_val = 1;
      return res;
}

static struct desc _thunk_2(void)
{     /* actual parameter at line 24 */
      struct desc res;
      register struct dsa_main_program_0 *dsa_0 = (void *)
         global_dsa->vector[0];
      dsa_0->line = 24;
      res.lval = 1;
      res.type = 'i';
      res.u.int_ptr = &(dsa_0->p_6);
      return res;
}

static void _sigma_2(void)
{     /* statement following 'do' at line 17 */
      register struct dsa_main_program_0 *dsa_0 = (void *)
         global_dsa->vector[0];
      /* start of local block 5 (level 4) at line 17 */
      dsa_0->old_top_4 = stack_top;
      dsa_0->new_top_4 = stack_top;
      dsa_0->line = 18;
      /* start of local block 6 (level 5) at line 19 */
      dsa_0->old_top_5 = stack_top;
      dsa_0->new_top_5 = stack_top;
      dsa_0->line = 20;
      dsa_0->p_6 = (*loc_int(dsa_0->prime_3, 1, dsa_0->j_3 + 50 * (
         dsa_0->k_3 - 1)));
      dsa_0->line = 21;
      if (!(less(dsa_0->p_6, 1000))) goto _omega_6;
      dsa_0->line = 21;
      ((global_dsa = (void *)dsa_0, outchar_0(make_arg((void *)_thunk_1,
          dsa_0), make_arg("0", NULL), make_arg((void *)_thunk_1, dsa_0)
         )));
_omega_6:
      dsa_0->line = 22;
      if (!(less(dsa_0->p_6, 100))) goto _omega_7;
      dsa_0->line = 22;
      ((global_dsa = (void *)dsa_0, outchar_0(make_arg((void *)_thunk_1,
          dsa_0), make_arg("0", NULL), make_arg((void *)_thunk_1, dsa_0)
         )));
_omega_7:
      dsa_0->line = 23;
      if (!(less(dsa_0->p_6, 10))) goto _omega_8;
      dsa_0->line = 23;
      ((global_dsa = (void *)dsa_0, outchar_0(make_arg((void *)_thunk_1,
          dsa_0), make_arg("0", NULL), make_arg((void *)_thunk_1, dsa_0)
         )));
_omega_8:
      dsa_0->line = 24;
      ((global_dsa = (void *)dsa_0, outinteger_0(make_arg((void *)
         _thunk_1, dsa_0), make_arg((void *)_thunk_2, dsa_0))));
      pop_stack(dsa_0->old_top_5);
      /* end of block 6 */
      pop_stack(dsa_0->old_top_4);
      /* end of block 5 */
      return;
}

static void _sigma_1(void)
{     /* statement following 'do' at line 15 */
      register struct dsa_main_program_0 *dsa_0 = (void *)
         global_dsa->vector[0];
      /* start of local block 4 (level 3) at line 15 */
      dsa_0->old_top_3 = stack_top;
      dsa_0->new_top_3 = stack_top;
      dsa_0->line = 16;
      dsa_0->line = 17;
      dsa_0->line = 17;
      dsa_0->k_3 = 1;
      dsa_0->teta_i_4 = 1;
_gamma_5:
      dsa_0->line = 17;
      if ((dsa_0->k_3 - (10)) * (dsa_0->teta_i_4 < 0 ? -1 : 
         dsa_0->teta_i_4 > 0 ? +1 : 0) > 0) goto _omega_5;
      global_dsa = (void *)dsa_0, _sigma_2();
      dsa_0->k_3 = dsa_0->teta_i_4 + dsa_0->k_3;
      goto _gamma_5;
_omega_5: /* element exhausted */
      dsa_0->line = 26;
      ((global_dsa = (void *)dsa_0, outstring_0(make_arg((void *)
         _thunk_1, dsa_0), make_arg("\n", NULL))));
      pop_stack(dsa_0->old_top_3);
      /* end of block 4 */
      return;
}

struct desc main_program_0 /* program */ (void)
{     struct dsa_main_program_0 my_dsa;
      register struct dsa_main_program_0 *dsa_0 = &my_dsa;
      my_dsa.proc = "main_program";
      my_dsa.file = "marst-2.7/examples/primes.alg";
      my_dsa.line = 1;
      my_dsa.parent = active_dsa, active_dsa = (struct dsa *)&my_dsa;
      my_dsa.vector[0] = (void *)dsa_0;
      /* start of procedure block 1 (level 0) at line 1 */
      dsa_0->old_top_0 = stack_top;
      dsa_0->new_top_0 = stack_top;
      /* start of local block 2 (level 1) at line 1 */
      dsa_0->old_top_1 = stack_top;
      dsa_0->new_top_1 = stack_top;
FirstFiveHundredPrimes_2:
      dsa_0->line = 2;
      /* start of local block 3 (level 2) at line 3 */
      dsa_0->old_top_2 = stack_top;
      /* jmp_buf must be of array type (ISO) */
      switch (setjmp(&dsa_0->jump_2[0]))
      {  case 0: break;
         case 1: pop_stack(dsa_0->new_top_2); active_dsa = (struct dsa 
         *)dsa_0; goto p2_3;
         case 2: pop_stack(dsa_0->new_top_2); active_dsa = (struct dsa 
         *)dsa_0; goto p9_3;
         case 3: pop_stack(dsa_0->new_top_2); active_dsa = (struct dsa 
         *)dsa_0; goto p4_3;
         case 4: pop_stack(dsa_0->new_top_2); active_dsa = (struct dsa 
         *)dsa_0; goto p6_3;
         default: fault("internal error on global go to");
      }
      dsa_0->line = 4;
      dsa_0->prime_3 = alloc_array('i', 1, 1, 500);
      dsa_0->new_top_2 = stack_top;
p1_3:
      dsa_0->line = 5;
      (*loc_int(dsa_0->prime_3, 1, 1)) = 2;
      dsa_0->line = 5;
      dsa_0->n_3 = 3;
      dsa_0->line = 5;
      dsa_0->j_3 = 1;
p2_3:
      dsa_0->line = 6;
      dsa_0->j_3 = dsa_0->j_3 + 1;
      dsa_0->line = 6;
      (*loc_int(dsa_0->prime_3, 1, dsa_0->j_3)) = dsa_0->n_3;
p3_3:
      dsa_0->line = 7;
      if (!(equal(dsa_0->j_3, 500))) goto _omega_1;
      dsa_0->line = 7;
      goto p9_3;
_omega_1:
p4_3:
      dsa_0->line = 8;
      dsa_0->n_3 = dsa_0->n_3 + 2;
p5_3:
      dsa_0->line = 9;
      dsa_0->k_3 = 2;
p6_3:
      dsa_0->line = 10;
      dsa_0->q_3 = dsa_0->n_3 / (*loc_int(dsa_0->prime_3, 1, dsa_0->k_3)
         );
      dsa_0->line = 10;
      dsa_0->r_3 = dsa_0->n_3 - dsa_0->q_3 * (*loc_int(dsa_0->prime_3, 
         1, dsa_0->k_3));
      dsa_0->line = 11;
      if (!(equal(dsa_0->r_3, 0))) goto _omega_2;
      dsa_0->line = 11;
      goto p4_3;
_omega_2:
p7_3:
      dsa_0->line = 12;
      if (!(notgreater(dsa_0->q_3, (*loc_int(dsa_0->prime_3, 1, 
         dsa_0->k_3))))) goto _omega_3;
      dsa_0->line = 12;
      goto p2_3;
_omega_3:
p8_3:
      dsa_0->line = 13;
      dsa_0->k_3 = dsa_0->k_3 + 1;
      dsa_0->line = 13;
      goto p6_3;
p9_3:
      dsa_0->line = 14;
      ((global_dsa = (void *)dsa_0, outstring_0(make_arg((void *)
         _thunk_1, dsa_0), make_arg("First Five Hundred Primes\n", NULL)
         )));
      dsa_0->line = 15;
      dsa_0->line = 15;
      dsa_0->j_3 = 1;
      dsa_0->teta_i_3 = 1;
_gamma_4:
      dsa_0->line = 15;
      if ((dsa_0->j_3 - (50)) * (dsa_0->teta_i_3 < 0 ? -1 : 
         dsa_0->teta_i_3 > 0 ? +1 : 0) > 0) goto _omega_4;
      global_dsa = (void *)dsa_0, _sigma_1();
      dsa_0->j_3 = dsa_0->teta_i_3 + dsa_0->j_3;
      goto _gamma_4;
_omega_4: /* element exhausted */
      pop_stack(dsa_0->old_top_2);
      /* end of block 3 */
      pop_stack(dsa_0->old_top_1);
      /* end of block 2 */
      pop_stack(dsa_0->old_top_0);
      /* end of block 1 */
      my_dsa.retval.lval = 0;
      my_dsa.retval.type = 0;
      active_dsa = my_dsa.parent;
      return my_dsa.retval;
}

int main(void)
{     /* Algol program startup code */
      main_program_0();
      return 0;
}

/* eof */

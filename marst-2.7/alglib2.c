/* alglib2.c */

/*----------------------------------------------------------------------
-- This file is part of GNU MARST, an Algol-to-C translator.
--
-- Copyright (C) 2000, 2001, 2002, 2007 Free Software Foundation, Inc.
--
-- This program is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
----------------------------------------------------------------------*/

#include "algol.h"

struct mem *stack_top = NULL;
struct dsa *active_dsa = NULL;
struct dsa *global_dsa = NULL;

/**********************************************************************/
/*                         AUXILIARY ROUTINES                         */
/**********************************************************************/

int real2int(double x)
{     /* converts from real to integer type */
      x = floor(x + 0.5);
      if (!((double)INT_MIN <= x && x <= (double)INT_MAX))
         fault("real number to be converted out of integer range");
      return (int)x;
}

/*--------------------------------------------------------------------*/

double int2real(int x)
{     /* converts from integer to real type */
      return (double)x;
}

/*--------------------------------------------------------------------*/

double expr(double x, double r)
{     /* rises real base x to real exponent r */
      double ret;
      if (x > 0.0)
         ret = pow(x, r);
      else if (x == 0.0 && r > 0.0)
         ret = 0.0;
      else
         fault("expr undefined " REAL_FMT, x);
      return ret;
}

/*--------------------------------------------------------------------*/

int expi(int i, int j)
{     /* rises integer base i to integer exponent j */
      int ret;
      unsigned u;
      if (j < 0 || (i == 0 && j == 0))
         fault("expi undefined %d", j);
      ret = 1;
      u = j;
      for (;;)
      {  if (u & 1) ret *= i;
         if (u >>= 1) i *= i; else break;
      }
      return ret;
}

/*--------------------------------------------------------------------*/

double expn(double x, int n)
{     /* rises real base x to integer exponent n */
      double ret;
      unsigned u;
      if (n == 0 && x == 0.0)
         fault("expn undefined " REAL_FMT, x);
      ret = 1.0;
      u = (n >= 0 ? n : (x = 1.0 / x, -n));
      for (;;)
      {  if (u & 1) ret *= x;
         if (u >>= 1) x *= x; else break;
      }
      return ret;
}

/*--------------------------------------------------------------------*/

double get_real(struct desc x)
{     /* returns value of real formal parameter called by name */
      double val;
      switch (x.type)
      {  case 'r':
            val = (x.lval ? *x.u.real_ptr : x.u.real_val);
            break;
         case 'i':
            val = int2real(x.lval ? *x.u.int_ptr : x.u.int_val);
            break;
         default:
            fault("evaluation of a real formal parameter not possible b"
               "ecause final actual parameter is not of arithmetic type"
               );
      }
      return val;
}

/*--------------------------------------------------------------------*/

int get_int(struct desc x)
{     /* returns value of integer formal parameter called by name */
      int val;
      switch (x.type)
      {  case 'r':
            val = real2int(x.lval ? *x.u.real_ptr : x.u.real_val);
            break;
         case 'i':
            val = (x.lval ? *x.u.int_ptr : x.u.int_val);
            break;
         default:
            fault("evaluation of an integer formal parameter not possib"
               "le because final actual parameter is not of arithmetic "
               "type");
      }
      return val;
}

/*--------------------------------------------------------------------*/

bool get_bool(struct desc x)
{     /* returns value of Boolean formal parameter called by name */
      bool val;
      switch (x.type)
      {  case 'b':
            val = (x.lval ? *x.u.bool_ptr : x.u.bool_val);
            break;
         default:
            fault("evaluation of a Boolean formal parameter not possibl"
               "e because final actual parameter is not of Boolean type"
               );
      }
      return val;
}

/*--------------------------------------------------------------------*/

struct label get_label(struct desc x)
{     /* returns value of formal label called by name */
      switch (x.type)
      {  case 'l':
            assert(!x.lval);
            break;
         default:
            fault("evaluation of a formal label not possible because fi"
               "nal actual parameter is not of label type");
      }
      return x.u.label;
}

/*--------------------------------------------------------------------*/

double set_real(struct desc x, double val)
{     /* assigns value to real formal parameter called by name */
      if (!x.lval)
         fault("assignment to a real formal parameter called by name no"
            "t possible because final actual parameter is not a variabl"
            "e");
      switch (x.type)
      {  case 'r':
            *x.u.real_ptr = val;
            break;
         case 'i':
            *x.u.int_ptr = real2int(val);
            break;
         default:
            fault("assignment to a real formal parameter called by name"
               " not possible because final actual parameter is not of "
               "arithmetic type");
      }
      return val;
}

/*--------------------------------------------------------------------*/

int set_int(struct desc x, int val)
{     /* assigns value to integer formal parameter called by name */
      if (!x.lval)
         fault("assignment to an integer formal parameter called by nam"
            "e not possible because final actual parameter is not a var"
            "iable");
      switch (x.type)
      {  case 'r':
            *x.u.real_ptr = int2real(val);
            break;
         case 'i':
            *x.u.int_ptr = val;
            break;
         default:
            fault("assignment to an integer formal parameter called by "
               "name not possible because final actual parameter is not"
               " of arithmetic type");
      }
      return val;
}

/*--------------------------------------------------------------------*/

bool set_bool(struct desc x, bool val)
{     /* assigns value to Boolean formal parameter called by name */
      if (!x.lval)
         fault("assignment to a Boolean formal parameter called by name"
            " not possible because final actual parameter is not a vari"
            "able");
      switch (x.type)
      {  case 'b':
            *x.u.bool_ptr = val;
            break;
         default:
            fault("assignment to a Boolean formal parameter called by n"
               "ame not possible because final actual parameter is not "
               "of Boolean type");
      }
      return val;
}

/*--------------------------------------------------------------------*/

struct arg make_arg(void *arg1, void *arg2)
{     /* makes actual parameter in unified form */
      struct arg arg;
      arg.arg1 = arg1;
      arg.arg2 = arg2;
      return arg;
}

/*--------------------------------------------------------------------*/

struct label make_label(void *jump, int kase)
{     /* makes "value of label" in form as used by go_to routine */
      struct label x;
      x.jump = jump;
      x.kase = kase;
      return x;
}

/*--------------------------------------------------------------------*/

void go_to(struct label x)
{     /* performs global go to */
      longjmp(x.jump, x.kase);
      /* no return */
}

/**********************************************************************/
/*                         ARRAY ROUTINES                             */
/**********************************************************************/

static void *my_malloc(int size)
{     /* allocates memory area of size bytes long */
      void *ptr;
      assert(size > 0);
      ptr = malloc(size);
      if (ptr == NULL)
         fault("main storage requested not available");
      return ptr;
}

/*--------------------------------------------------------------------*/

static void my_free(void *ptr)
{     /* frees memory area specified by pointer ptr */
      assert(ptr != NULL);
      free(ptr);
      return;
}

/*--------------------------------------------------------------------*/

static void *push_stack(int size)
{     /* allocates free memory block of size bytes long and pushes this
         block to the stack */
      struct mem *top;
      assert(size > 0);
      if (size > INT_MAX - offsetof(struct mem, body))
         fault("main storage requested not available");
      top = my_malloc(offsetof(struct mem, body) + size);
      top->size = size;
      top->ptr = stack_top;
      stack_top = top;
      return &top->body;
}

/*--------------------------------------------------------------------*/

void pop_stack(struct mem *top)
{     /* pulls from the stack and frees memory blocks until in top of
         the stack memory block pointed to by top is not be proved */
      while (stack_top != top)
      {  struct mem *ptr;
         ptr = stack_top;
         stack_top = stack_top->ptr;
         my_free(ptr);
      }
      return;
}

/*--------------------------------------------------------------------*/

static struct dv *copy_dv(int own, struct dv *dope)
{     /* creates copy of array dope vector */
      struct dv *dv;
      int size;
      assert(1 <= dope->n && dope->n <= 9);
      size = offsetof(struct dv, d[dope->n]);
      dv = own ? my_malloc(size) : push_stack(size);
      memcpy(dv, dope, size);
      return dv;
}

/*--------------------------------------------------------------------*/

static struct dv *make_dv(int own, int n, va_list arg)
{     /* creates array dope vector using subscripts bound */
      struct dv dv;
      int k;
      assert(1 <= n && n <= 9);
      dv.base = NULL;
      dv.n = n;
      for (k = 0; k < n; k++)
      {  dv.d[k].lo = va_arg(arg, int);
         dv.d[k].up = va_arg(arg, int);
      }
      return copy_dv(own, &dv);
}

/*--------------------------------------------------------------------*/

static int array_size(int type, struct dv *dv)
{     /* determines array size in bytes */
      int size, k;
      switch (type)
      {  case 'r': size = sizeof(double); break;
         case 'i': size = sizeof(int); break;
         case 'b': size = sizeof(bool); break;
         default:  assert(type != type);
      }
      for (k = 0; k < dv->n; k++)
         if (dv->d[k].lo > dv->d[k].up) return 0;
      for (k = 0; k < dv->n; k++)
      {  int lo = dv->d[k].lo;
         int up = dv->d[k].up;
         int t;
         /* check up - lo + 1 <= INT_MAX to prevent overflow */
         if (lo < 0 && up > (INT_MAX + lo) - 1)
too:        fault("unable to allocate too long array");
         t = up - lo + 1;
         /* check size * t < INT_MAX to prevent overflow */
         if (size > INT_MAX / t) goto too;
         size *= t;
      }
      return size;
}

/*--------------------------------------------------------------------*/

struct dv *alloc_array(int type, int n, ...)
{     /* creates local array and returns pointer to its dope vector */
      /* type defines array type ('r' - real, 'i' - integer, 'b' - Boo-
         lean) */
      /* n is number of subscript bounds */
      /* variable parameter list defines n pairs in the form (l, u),
         where l and u are lower and upper subscript bounds */
      struct dv *dv;
      int size;
      va_list arg;
      va_start(arg, n);
      dv = make_dv(0, n, arg);
      va_end(arg);
      size = array_size(type, dv);
      dv->base = size == 0 ? NULL : push_stack(size);
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *alloc_same(int type, struct dv *dope)
{     /* creates local array that has the same dimension and subscript
         bounds as specified by dope and returns pointer to its dope
         vector */
      struct dv *dv;
      int size;
      dv = copy_dv(0, dope);
      size = array_size(type, dv);
      dv->base = size == 0 ? NULL : push_stack(size);
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *own_array(int type, int n, ...)
{     /* creates own array and returns pointer to its dope vector
         (each own array is created when the first entrance to the
         appropriate block is occured) */
      /* parameter list has the same meaning as for alloc_array */
      struct dv *dv;
      int size;
      va_list arg;
      va_start(arg, n);
      dv = make_dv(1, n, arg);
      va_end(arg);
      size = array_size(type, dv);
      dv->base = size == 0 ? NULL : my_malloc(size);
      /* initialize own array */
      if (size != 0) memset(dv->base, 0, size);
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *own_same(int type, struct dv *dope)
{     /* creates own array that has the same dimension and subscript
         bounds as specified by dope and returns pointer to its dope
         vector */
      struct dv *dv;
      int size;
      dv = copy_dv(1, dope);
      size = array_size(type, dv);
      dv->base = size == 0 ? NULL : my_malloc(size);
      /* initialize own array */
      if (size != 0) memset(dv->base, 0, size);
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *copy_real(struct arg arg)
{     /* creates copy of real formal array called by value */
      struct dv *dope = arg.arg1; /* dope vector of actual array */
      int type = (int)arg.arg2;   /* type of actual array */
      struct dv *dv;
      int size;
      dv = copy_dv(0, dope);
      size = array_size('r', dv);
      dv->base = size == 0 ? NULL : push_stack(size);
      if (type == 'r')
      {  /* actual array is of real type */
         if (size != 0) memcpy(dv->base, dope->base, size);
      }
      else if (type == 'i')
      {  /* actual array is of integer type (conversion is needed) */
         int *s;
         double *t;
         for (s = dope->base, t = dv->base; size > 0;
            s++, t++, size -= sizeof(double)) *t = int2real(*s);
      }
      else
         fault("creation of a real formal array called by value not pos"
            "sible because final actual parameter is not an array of ar"
            "ithmetic type");
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *copy_int(struct arg arg)
{     /* creates copy of integer formal array called by value */
      struct dv *dope = arg.arg1; /* dope vector of actual array */
      int type = (int)arg.arg2;   /* type of actual array */
      struct dv *dv;
      int size;
      dv = copy_dv(0, dope);
      size = array_size('i', dv);
      dv->base = size == 0 ? NULL : push_stack(size);
      if (type == 'r')
      {  /* actual array is of real type (conversion is needed) */
         double *s;
         int *t;
         for (s = dope->base, t = dv->base; size > 0;
            s++, t++, size -= sizeof(int)) *t = real2int(*s);
      }
      else if (type == 'i')
      {  /* actual array is of integer type */
         if (size != 0) memcpy(dv->base, dope->base, size);
      }
      else
         fault("creation of an integer formal array called by value not"
            " possible because final actual parameter is not an array o"
            "f arithmetic type");
      return dv;
}

/*--------------------------------------------------------------------*/

struct dv *copy_bool(struct arg arg)
{     /* creates copy of Boolean formal array called by value */
      struct dv *dope = arg.arg1; /* dope vector of actual array */
      int type = (int)arg.arg2;   /* type of actual array */
      struct dv *dv;
      int size;
      dv = copy_dv(0, dope);
      size = array_size('b', dv);
      dv->base = size == 0 ? NULL : push_stack(size);
      if (type == 'b')
      {  /* actual array is of Boolean type */
         if (size != 0) memcpy(dv->base, dope->base, size);
      }
      else
         fault("creation of a Boolean formal array called by value not "
            "possible because final actual parameter is not an array of"
            " Boolean type");
      return dv;
}

/*--------------------------------------------------------------------*/

static int loc_elem(struct dv *dv, int n, va_list arg)
{     /* computes rolled subscript of array element as if array were
         one-dimensional with lower bound of subscript equal to zero */
      /* dv points to array dope vector */
      /* n is actual number of subscripts in subscripted variable */
      /* variable parameter list defined values of subscripts */
      int loc, k;
      if (dv->n != n)
         fault("unequal number of dimensions for actual and formal para"
            "meter array");
      loc = 0;
      for (k = 0; k < n; k++)
      {  int lo = dv->d[k].lo;
         int up = dv->d[k].up;
         int i = va_arg(arg, int);
         if (!(lo <= i && i <= up))
            fault("value of subscript expression not within declared bo"
               "unds of array");
         loc = (up - lo + 1) * loc + (i - lo);
      }
      return loc;
}

/*--------------------------------------------------------------------*/

double *loc_real(struct dv *dv, int n, ...)
{     /* returns address of subscripted variable of real type */
      va_list arg;
      int loc;
      va_start(arg, n);
      loc = loc_elem(dv, n, arg);
      va_end(arg);
      return (double *)dv->base + loc;
}

/*--------------------------------------------------------------------*/

int *loc_int(struct dv *dv, int n, ...)
{     /* returns address of subscripted variable of integer type */
      va_list arg;
      int loc;
      va_start(arg, n);
      loc = loc_elem(dv, n, arg);
      va_end(arg);
      return (int *)dv->base + loc;
}

/*--------------------------------------------------------------------*/

bool *loc_bool(struct dv *dv, int n, ...)
{     /* returns address of subscripted variable of Boolean type */
      va_list arg;
      int loc;
      va_start(arg, n);
      loc = loc_elem(dv, n, arg);
      va_end(arg);
      return (bool *)dv->base + loc;
}

/**********************************************************************/
/*                       INPUT/OUTPUT ROUTINES                        */
/**********************************************************************/

#define CHANNEL_MAX 16
/* maximum allowed number of input/output channels */

static FILE *stream[CHANNEL_MAX];
/* streams to simulate input/output channels */

static int status[CHANNEL_MAX];
/* current statuses of input/output channels (0 - closed, 'r' - open
   for reading, 'w' - open for writing) */

/*--------------------------------------------------------------------*/

void fault(char *fmt, ...)
{     /* prints fatal error message and terminates execution */
      va_list arg;
      struct dsa *dsa;
      fprintf(stderr, "\n");
      fprintf(stderr, "fault: ");
      va_start(arg, fmt);
      vfprintf(stderr, fmt, arg);
      va_end(arg);
      fprintf(stderr, "\n");
      /* print extended run-time diagnostics */
      for (dsa = active_dsa; dsa != NULL; dsa = dsa->parent)
      {  if (dsa->parent == NULL)
            fprintf(stderr, "main program");
         else
            fprintf(stderr, "procedure %s", dsa->proc);
         fprintf(stderr, ", file %s, line %d\n", dsa->file, dsa->line);
      }
      /* flush all channels open for writing */
      fflush(stderr);
      {  int k;
         for (k = 0; k < CHANNEL_MAX; k++)
            if (status[k] == 'w') fflush(stream[k]);
      }
      /* and exit to the control program */
      exit(EXIT_FAILURE);
      /* no return */
}

/*--------------------------------------------------------------------*/

static void connect(int channel, int mode)
{     /* connects channel to external file */
      /* mode is 'r' for reading and 'w' for writing */
      if (!(0 <= channel && channel < CHANNEL_MAX))
         fault("channel number %d out of range", channel);
      assert(mode == 'r' || mode == 'w');
      if (status[channel] != mode)
      {  if (channel == 0)
         {  if (mode == 'w')
               fault("output to standard input channel not allowed");
            if (status[channel] == 0)
            {  stream[channel] = stdin;
               status[channel] = 'r';
            }
         }
         else if (channel == 1)
         {  if (mode == 'r')
               fault("input from standard output channel not allowed");
            if (status[channel] == 0)
            {  stream[channel] = stdout;
               status[channel] = 'w';
            }
         }
         else
         {  char dd_name[15+1], *filename;
            if (status[channel] != 0) fclose(stream[channel]);
            sprintf(dd_name, "FILE_%d", channel);
            filename = getenv(dd_name);
            if (filename == NULL) filename = dd_name;
            stream[channel] = fopen(filename, mode == 'w' ? "w" : "r");
            if (stream[channel] == NULL)
               fault("unable to connect channel %d to file `%s' for %s "
                  "- %s", channel, filename,
                  mode == 'w' ? "output" : "input", strerror(errno));
            status[channel] = mode;
         }
      }
      return;
}

/*--------------------------------------------------------------------*/

int inchar(int channel)
{     /* reads character from channel */
      int c;
      connect(channel, 'r');
      c = fgetc(stream[channel]);
      if (ferror(stream[channel]))
         fault("unable to input from channel %d - %s", channel,
            strerror(errno));
      if (feof(stream[channel]))
         fault("unable to input from channel %d - input request beyond "
            "end of data", channel);
      return c;
}

/*--------------------------------------------------------------------*/

void outchar(int channel, int c)
{     /* writes character to channel */
      connect(channel, 'w');
      fputc(c, stream[channel]);
      if (ferror(stream[channel]))
         fault("unable to output to channel %d - %s", channel,
            strerror(errno));
      return;
}

/*--------------------------------------------------------------------*/

void outstring(int channel, char *str)
{     /* writes character string to channel */
      connect(channel, 'w');
      fprintf(stream[channel], "%s", str);
      return;
}

/*--------------------------------------------------------------------*/

static char *input_data(int channel)
{     /* reads data item from channel */
      static char str[255+1];
      int len = 0, c;
      /* skip non-significant characters */
      for (;;)
      {  c = inchar(channel);
         if (!isspace(c)) break;
      }
      /* data item is a sequence of significant characters */
      while (!isspace(c))
      {  if (len == sizeof(str) - 1)
         {  str[len] = '\0';
            fault("input data item `%.12s...' too long", str);
         }
         str[len++] = (char)c;
         c = inchar(channel);
      }
      str[len] = '\0';
      return str;
}

/*--------------------------------------------------------------------*/

int ininteger(int channel)
{     /* reads quantity of integer type from channel */
      int x;
      char *str, *ptr;
      str = input_data(channel);
      x = strtol(str, &ptr, 10);
      if (*ptr != '\0')
         fault("unable to convert `%s' to integer number", str);
      return x;
}

/*--------------------------------------------------------------------*/

void outinteger(int channel, int val)
{     /* writes quantity of integer type to channel */
      connect(channel, 'w');
      fprintf(stream[channel], "%d ", val);
      return;
}

/*--------------------------------------------------------------------*/

double inreal(int channel)
{     /* reads quantity of real type from channel */
      double x;
      char *str, *ptr;
      str = input_data(channel);
      x = strtod(str, &ptr);
      if (*ptr != '\0')
         fault("unable to convert `%s' to real number", str);
      return x;
}

/*--------------------------------------------------------------------*/

void outreal(int channel, double val)
{     /* writes quantity of real type to channel */
      connect(channel, 'w');
      fprintf(stream[channel], REAL_FMT " ", val);
      return;
}

/* eof */

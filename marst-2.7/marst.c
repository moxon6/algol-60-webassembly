/* marst.c */

/*----------------------------------------------------------------------
-- This file is part of GNU MARST, an Algol-to-C translator.
--
-- Copyright (C) 2000, 2001, 2002, 2007, 2013 Free Software Foundation,
-- Inc.
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

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*----------------------------------------------------------------------
--                      Quod si digna tua minus est mea pagina laude
--                      At voluisse sat est: animum, non carmina jacto.
--
--
-- This program is a translator. It translates Algol 60 programs to the
-- C programming language.
--
-- Input language of the translator is hardware representation of the
-- reference language Algol 60 as described in IFIP document:
--
-- Modified Report on the Algorithmic Language ALGOL 60. The Computer
-- Journal, Vol. 19, No. 4, Nov. 1976, pp. 364-379. (This document is
-- an official IFIP standard document. It is NOT a part of the MARST
-- package.)
--
-- Note that there are some differences between the Revised Report and
-- the Modified Report since the latter is a result of application of
-- the following IFIP document to the Revised Report:
--
-- R.M.De Morgan, I.D.Hill, and B.A.Wichmann. A Supplement to the
-- ALGOL 60 Revised Report. The Computer Journal, Vol. 19, No. 3, 1976,
-- pp. 276-288. (This document is an official IFIP standard document.
-- It is NOT a part of the MARST package.)
--
-- The translator as well as library routines are programmed in basic
-- dialect of the C programming language which is a subset of ISO C as
-- described in preliminary ISO document (this dialect is practically
-- the same as widely known 1989' ANSI C):
--
-- ISO, Programming languages - C, WG14/N843, Commitee Draft - August 3,
-- 1998. (This document is an official ISO standard document. It is NOT
-- a part of the MARST package.)
--
-- The output code produced by the translator is on the same dialect.
--
-- To translate source Algol 60 program the translator uses so called
-- recursive descent technique for parsing. This technique means that
-- the parsing tree is (implicitly) built from its root to its leaves.
-- Each parsing routine builds subtree the root of which corresponds to
-- the appropriate non-terminal.
--
-- The translation is performed in two passes. The first pass is used
-- to determine structure of blocks and to accumulate information about
-- identifiers. On the second pass source program is read once again
-- and the syntax directed translation is used to generate output code.
--
-- The syntax listed below in the description of each parsing routine
-- and which is used by these routines is not the original syntax of the
-- reference language. But this syntax is equivalent to the original one
-- in the sense that they both define the same language.
----------------------------------------------------------------------*/

static char *version = "MARST -- Algol-to-C Translator, Version 2.7";
/* Version information. */

static char *infilename = "";
/* The name of input text file that contains source Algol 60 program.
   If this file is the standard input file then its name is "(stdin)".
   This name used in error and warning messages. */

static FILE *infile;
/* The stream connected to the input text file. */

static char *outfilename = "";
/* The name of output text file to which the translator emits output
   C code. If this file is the standard output file then its name is
   "(stdout)". */

static FILE *outfile;
/* The stream connected to the output text file. */

static int debug = 0;
/* Debug mode flag. If this flag is set then the translator emits all
   tokens of source program to the output text file as comments. */

static int errmax = 0;
/* Maximal error count. The translator stops after the specified number
   of errors has been detected. Zero means to continue translation until
   the end of the file. This value must be in range from 0 to 255. */

static int warn = 1;
/* Warning flag. If this flag is set then the translator send warning
   messages to stderr; otherwise all warning messages are suppressed */

static int width = 72;
/* Desirable line width used in output C code. This value must be in
   range from 50 to 255. */

static int time_stamp = 1;
/* Time stamp flag. If this flag is set then the translator writes date
   and time of translation to output C code as comment. */

static int first_pass;
/* First pass flag. This flag is set during the first pass and is clear
   during the second pass. */

static int second_pass;
/* Second pass flag. This flag is clear during the first pass and is set
   during the second pass. */

static int e_count = 0;
/* Error count. */

static int w_count = 0;
/* Warning count. */

static int l_count;
/* Source line count. This value is increased by one each time when the
   next line has been read from the input text file. */

static int l_maxlen = 100;
/* Maximal length of source line (excluding '\0'). This value will be
   increased automatically when it will be necessary. */

static char *line /* [l_maxlen+1] */;
/* Current source line (i.e. line of source Algol 60 program) read from
   the input text file. */

static int pos;
/* Position of the current (i.e. not processed yet) character in the
   current source line. This value is changed from 0 to l_maxlen. */

static int symbol;
/* Code of the current basic symbol: */

#define S_EOF        0  /* _|_ */
#define S_LETTER     1  /* a, b, ..., z, A, B, ..., Z */
#define S_DIGIT      2  /* 0, 1, ..., 9 */
#define S_PLUS       3  /* + */
#define S_MINUS      4  /* - */
#define S_TIMES      5  /* * */
#define S_SLASH      6  /* / */
#define S_INTDIV     7  /* % */
#define S_POWER      8  /* ^, ** */
#define S_LESS       9  /* < */
#define S_NOTGREATER 10 /* <= */
#define S_EQUAL      11 /* = */
#define S_NOTLESS    12 /* >= */
#define S_GREATER    13 /* > */
#define S_NOTEQUAL   14 /* != */
#define S_EQUIV      15 /* == */
#define S_IMPL       16 /* -> */
#define S_OR         17 /* | */
#define S_AND        18 /* & */
#define S_NOT        19 /* ! */
#define S_COMMA      20 /* , */
#define S_POINT      21 /* . */
#define S_TEN        22 /* # */
#define S_COLON      23 /* : */
#define S_SEMICOLON  24 /* ; */
#define S_ASSIGN     25 /* := */
#define S_LEFT       26 /* ( */
#define S_RIGHT      27 /* ) */
#define S_BEGSUB     28 /* [ */
#define S_ENDSUB     29 /* ] */
#define S_OPEN       30 /* " (opening quote) */
#define S_CLOSE      31 /* " (closing quote, not used at this time) */
#define S_ARRAY      32 /* array */
#define S_BEGIN      33 /* begin */
#define S_BOOLEAN    34 /* Boolean, boolean */
#define S_CODE       35 /* code */
#define S_COMMENT    36 /* comment */
#define S_DO         37 /* do */
#define S_ELSE       38 /* else */
#define S_END        39 /* end */
#define S_FALSE      40 /* false */
#define S_FOR        41 /* for */
#define S_GOTO       42 /* go to, goto */
#define S_IF         43 /* if */
#define S_INTEGER    44 /* integer */
#define S_LABEL      45 /* label */
#define S_OWN        46 /* own */
#define S_PROCEDURE  47 /* procedure */
#define S_REAL       48 /* real */
#define S_STEP       49 /* step */
#define S_STRING     50 /* string */
#define S_SWITCH     51 /* switch */
#define S_THEN       52 /* then */
#define S_TRUE       53 /* true */
#define S_UNTIL      54 /* until */
#define S_VALUE      55 /* value */
#define S_WHILE      56 /* while */

static int s_char;
/* Value of the current basic symbol. It has sense only for S_LETTER and
   S_DIGIT. */

/*----------------------------------------------------------------------
-- my_assert - perform diagnostics.
--
-- This routine is used indirectly through 'assert' macro and checks
-- some error conditions that can happen only if there is a bug in the
-- translator. */

static void my_assert(char *expr, char *file, int line)
{     fprintf(stderr,
         "Internal translator error: %s, file %s, line %d\n",
         expr, file, line);
      fprintf(stderr, "Please, report to <bug-marst@gnu.org>\n");
      exit(EXIT_FAILURE);
      /* no return */
}

#undef assert

#define assert(expr)\
      ((void)((expr) || (my_assert(#expr, __FILE__, __LINE__), 1)))

/*----------------------------------------------------------------------
-- my_malloc - allocate memory area.
--
-- This routine allocates a memory area of size bytes long and returns
-- a pointer to the beginning of this area. It is a fatal error if this
-- request cannot be satisfied. */

static void *my_malloc(int size)
{     void *ptr;
      assert(size > 0);
      ptr = malloc(size);
      if (ptr == NULL)
      {  fprintf(stderr, "Main storage requested not available\n");
         exit(EXIT_FAILURE);
      }
      return ptr;
}

#undef malloc

#define malloc ??? /* further usage is verboten */

/*----------------------------------------------------------------------
-- my_realloc - reallocate memory area.
--
-- This routine reallocates a memory area pointed to by ptr to increase
-- it to size bytes long and returns pointer to reallocated area. It is
-- a fatal error if this request cannot be satisfied. */

static void *my_realloc(void *ptr, int size)
{     assert(ptr != NULL && size > 0);
      ptr = realloc(ptr, size);
      if (ptr == NULL)
      {  fprintf(stderr, "Main storage requested not available\n");
         exit(EXIT_FAILURE);
      }
      return ptr;
}

#undef realloc

#define realloc ??? /* further usage is verboten */

/*----------------------------------------------------------------------
-- my_free - free memory area.
--
-- This routine frees memory area pointed to by ptr. */

static void my_free(void *ptr)
{     assert(ptr != NULL);
      free(ptr);
      return;
}

#undef free

#define free ??? /* further usage is verboten */

/*----------------------------------------------------------------------
-- error - print error message.
--
-- This routine formats and sends to stderr a message that reflects an
-- error in source Algol 60 program. */

static void error(char *msg, ...)
{     va_list arg;
      fprintf(stderr, "%s:%d: ", infilename, l_count);
      va_start(arg, msg);
      vfprintf(stderr, msg, arg);
      va_end(arg);
      fprintf(stderr, "\n");
      if (debug && first_pass)
      {  fprintf(outfile, ">>%s:%d: ", infilename, l_count);
         va_start(arg, msg);
         vfprintf(outfile, msg, arg);
         va_end(arg);
         fprintf(outfile, "\n");
      }
      e_count++;
      if (e_count == errmax)
      {  error("too many errors detected; translation terminated");
         exit(EXIT_FAILURE);
      }
      return;
}

/*----------------------------------------------------------------------
-- warning - print warning message.
--
-- This routine formats and sends to stderr a message that reflects
-- some potential error or usage of some non-standard feature in source
-- Algol 60 program. */

static void warning(char *msg, ...)
{     if (first_pass && warn)
      {  va_list arg;
         fprintf(stderr, "%s:%d: warning: ", infilename, l_count);
         va_start(arg, msg);
         vfprintf(stderr, msg, arg);
         va_end(arg);
         fprintf(stderr, "\n");
         if (debug)
         {  fprintf(outfile, ">>%s:%d: warning: ", infilename, l_count);
            va_start(arg, msg);
            vfprintf(outfile, msg, arg);
            va_end(arg);
            fprintf(outfile, "\n");
         }
         w_count++;
      }
      return;
}

/*----------------------------------------------------------------------
-- read_line - read the next source line.
--
-- This routine scans the input text file char by char until '\n' to
-- build the next line of source Algol 60 program. If the next line has
-- been built, then the routine returns zero; if not (due to the end of
-- file), then the routine places an empty string as the next line and
-- returns non-zero. */

static int read_line(void)
{     int c, len = 0;
      for (;;)
      {  c = fgetc(infile);
         if (ferror(infile))
         {  fprintf(stderr, "Read error on `%s' - %s\n", infilename,
               strerror(errno));
            exit(EXIT_FAILURE);
         }
         if (feof(infile))
         {  if (len > 0)
            {  l_count++;
               warning("missing final newline");
               l_count--;
               break;
            }
            line[0] = '\0';
            return 1; /* end of file reached */
         }
         if (c == '\n') break;
         if (iscntrl(c) && !isspace(c))
         {  l_count++;
            error("invalid control character 0x%02X", c);
            l_count--;
            c = ' ';
         }
         if (len == l_maxlen)
         {  /* line too long; increase buffer size */
            l_maxlen += l_maxlen;
            line = my_realloc(line, l_maxlen + 1);
         }
         line[len++] = (char)c;
      }
      /* end of line reached */
      line[len] = '\0';
      l_count++;
      return 0;
}

/*----------------------------------------------------------------------
-- skip_pad - skip non-significant characters.
--
-- This routine scans the current source line to skip all characters
-- that are non-significant. If the end of line occurs then the routine
-- automatically reads the next source line. After return the current
-- characters will be the first significant one. */

static void skip_pad(void)
{     for (;;)
      {  if (line[pos] == '\0')
         {  /* end of line reached */
            if (read_line()) line[0] = 0x1A; /* end of file reached */
            pos = 0;
         }
         else if (isspace(line[pos]))
         {  /* ignore non-significant character */
            pos++;
         }
         else
         {  /* significant character detected */
            break;
         }
      }
      return;
}

/*----------------------------------------------------------------------
-- scan_symbol - scan basic symbol.
--
-- This routine scans the next basic symbol. The code of scanned symbol
-- is assigned to 'symbol' and its value (if the symbol is a letter or
-- a digit) is assigned to 's_char'. */

#define TEN_CHAR '#'
/* character used to denote decimal exponent (subscripted ten) */

#define check_word(len, word, code)\
if (!memcmp(line+pos, word, len) && !isalnum(line[pos+len]))\
{     symbol = code, pos += len;\
      break;\
}

#define check_spec(c, code1, code2)\
if (line[pos+1] != c)\
      symbol = code1, pos++;\
else\
      symbol = code2, pos += 2;

static void scan_symbol(void)
{     /* skip optional non-significant characters that can precede to
         basic symbol */
scan: skip_pad();
      /* now the current character is significant */
      if (isalpha(line[pos]))
      {  /* letter */
         /* a sequence of letters is recognized as keyword if and only
            if there are no letters and digits immediately preceding or
            following that sequence */
         if (pos > 0 && isalnum(line[pos-1])) goto alfa;
         switch (line[pos])
         {  case 'a':
               check_word(5, "array",     S_ARRAY);
               goto alfa;
            case 'b':
               check_word(5, "begin",     S_BEGIN);
               check_word(7, "boolean",   S_BOOLEAN);
               goto alfa;
            case 'B':
               check_word(7, "Boolean",   S_BOOLEAN);
               goto alfa;
            case 'c':
               check_word(4, "code",      S_CODE);
               check_word(7, "comment",   S_COMMENT);
               goto alfa;
            case 'd':
               check_word(2, "do",        S_DO);
               goto alfa;
            case 'e':
               check_word(4, "else",      S_ELSE);
               check_word(3, "end",       S_END);
               goto alfa;
            case 'f':
               check_word(5, "false",     S_FALSE);
               check_word(3, "for",       S_FOR);
               goto alfa;
            case 'g':
               check_word(4, "goto",      S_GOTO);
               check_word(5, "go to",     S_GOTO);
               check_word(6, "go  to",    S_GOTO);
               check_word(7, "go   to",   S_GOTO);
               goto alfa;
            case 'i':
               check_word(2, "if",        S_IF);
               check_word(7, "integer",   S_INTEGER);
               goto alfa;
            case 'l':
               check_word(5, "label",     S_LABEL);
               goto alfa;
            case 'o':
               check_word(3, "own",       S_OWN);
               goto alfa;
            case 'p':
               check_word(9, "procedure", S_PROCEDURE);
               goto alfa;
            case 'r':
               check_word(4, "real",      S_REAL);
               goto alfa;
            case 's':
               check_word(4, "step",      S_STEP);
               check_word(6, "string",    S_STRING);
               check_word(6, "switch",    S_SWITCH);
               goto alfa;
            case 't':
               check_word(4, "then",      S_THEN);
               check_word(4, "true",      S_TRUE);
               goto alfa;
            case 'u':
               check_word(5, "until",     S_UNTIL);
               goto alfa;
            case 'v':
               check_word(5, "value",     S_VALUE);
               goto alfa;
            case 'w':
               check_word(5, "while",     S_WHILE);
               goto alfa;
            default:
alfa:          symbol = S_LETTER, s_char = (unsigned char)line[pos++];
         }
      }
      else if (isdigit(line[pos]))
      {  /* digit */
         symbol = S_DIGIT, s_char = (unsigned char)line[pos++];
      }
      else
      {  /* special character */
         switch (line[pos])
         {  case 0x1A:
               symbol = S_EOF; break;
            case '+':
               symbol = S_PLUS, pos++; break;
            case '-':
               check_spec('>', S_MINUS, S_IMPL); break;
            case '*':
               check_spec('*', S_TIMES, S_POWER); break;
            case '/':
               symbol = S_SLASH, pos++; break;
            case '%':
               symbol = S_INTDIV, pos++; break;
            case '^':
               symbol = S_POWER, pos++; break;
            case '<':
               check_spec('=', S_LESS, S_NOTGREATER); break;
            case '=':
               check_spec('=', S_EQUAL, S_EQUIV); break;
            case '>':
               check_spec('=', S_GREATER, S_NOTLESS); break;
            case '!':
               check_spec('=', S_NOT, S_NOTEQUAL); break;
            case '|':
               symbol = S_OR, pos++; break;
            case '&':
               symbol = S_AND, pos++; break;
            case ',':
               symbol = S_COMMA, pos++; break;
            case '.':
               symbol = S_POINT, pos++; break;
            case TEN_CHAR:
               symbol = S_TEN, pos++; break;
            case ':':
               check_spec('=', S_COLON, S_ASSIGN); break;
            case ';':
               symbol = S_SEMICOLON, pos++; break;
            case '(':
               symbol = S_LEFT, pos++; break;
            case ')':
               symbol = S_RIGHT, pos++; break;
            case '[':
               symbol = S_BEGSUB, pos++; break;
            case ']':
               symbol = S_ENDSUB, pos++; break;
            case '"':
               symbol = S_OPEN, pos++; break;
            default:
               error("invalid character `%c'", line[pos]);
               pos++;
               goto scan; /* ignore it */
         }
      }
      return;
}

/*----------------------------------------------------------------------
-- The structure declared below defines a token (elementary syntactic
-- unit) of source Algol 60 program. The grammar describing the input
-- language and which is used by parsing routines is LL(2). This means
-- that in general case the translator needs two tokens: the current
-- token and the next one. In some cases the translator also needs the
-- token preceding the current token. These tokens are placed in the
-- array 'token' (see declaration below) in the following manner:
--
-- token[0] is the token preceding the current token;
-- token[1] is the current (being processed) token;
-- token[2] is the token following the current token.
--
-- Should note that token[0] and token[1] are always defined, but
-- token[2] may be undefined, because most productions can be properly
-- resolved without token[2] hint. */

static int t_maxlen = 100;
/* Maximal length of token image (excluding '\0'); this value will be
   increased automatically when it will be necessary */

static struct
{     /* source program token */
      int ssn;
      /* source line number where token starts */
      int code;
      /* token code: */
#define T_UNDEF   0  /* token undefined (for token[2] only) */
#define T_IDENT   1  /* identifier */
#define T_INT     2  /* integer constant */
#define T_REAL    3  /* real constant */
#define T_FALSE   4  /* logical constant 'false' */
#define T_TRUE    5  /* logical constant 'true' */
#define T_STRING  6  /* character string */
#define T_DELIM   7  /* delimiter */
      int delim;
      /* delimiter code, i.e. code of corresponding basic symbol (has
         sense only for T_DELIM) */
      int len;
      /* current length if token image (excluding '\0') */
      char *image /* [t_maxlen+1] */;
      /* character string representing "image" of token */
} token[3];

/* Some handy-to-use macros. */
#define t_ssn           (token[1].ssn)
#define t_code          (token[1].code)
#define t_delim(what)   (t_code == T_DELIM && token[1].delim == what)
#define t_image         (token[1].image)

/*----------------------------------------------------------------------
-- add_char - add character to a token image.
--
-- This routine adds the character c to the image of token[k]. */

static void add_char(int k, int c)
{     if (token[k].len == t_maxlen)
      {  int kk;
         t_maxlen += t_maxlen;
         for (kk = 0; kk <= 2; kk++)
            token[kk].image =
               my_realloc(token[kk].image, t_maxlen + 1);
      }
      token[k].image[token[k].len++] = (char)c;
      token[k].image[token[k].len] = '\0';
      return;
}

/*----------------------------------------------------------------------
-- scan_token - scan the next token.
--
-- This routine scans the next token from the input file and places it
-- into token[k]. This routine also skips comment sequences that follows
-- 'comment' and 'end' delimiters.
--
-- Normally this routine is called with k = 1. Special call with k = 2
-- is used only to forescan a token that follows the current token. */

#define T_MAXLEN 100
/* Maximal length of character string representing identifier or number
   constant. (May be increased if necessary.) */

static void scan_token(int k)
{     assert(k == 1 || (k == 2 && token[2].code == T_UNDEF));
scan: /* skip optional comment sequence following 'end' symbol */
      if (token[k-1].code == T_DELIM && token[k-1].delim == S_END)
      {  int some = 0; /* something follows 'end' */
         int flag = 0; /* message displayed */
         for (;;)
         {  if (symbol == S_EOF || symbol == S_SEMICOLON ||
                symbol == S_ELSE || symbol == S_END) break;
            some = 1;
            if (!(symbol == S_LETTER || symbol == S_DIGIT ||
                  symbol == S_FALSE || symbol == S_TRUE) && !flag)
            {  warning("comment sequence following `end' contains delim"
                  "iter(s)");
               flag = 1;
            }
            scan_symbol();
         }
         if (symbol == S_EOF && some)
            warning("comment sequence following `end' terminated by eof"
               );
      }
      /* skip optional comment sequence following 'comment' symbol */
      {  int flag = 0; /* message displayed */
         while (symbol == S_COMMENT)
         {  if (!(token[k-1].code == T_DELIM &&
               (token[k-1].delim == S_SEMICOLON ||
                token[k-1].delim == S_BEGIN)))
            {  if (token[k-1].code == T_DELIM &&
                   token[k-1].delim == S_EOF)
               {  if (!flag)
                  {  warning("no symbols preceding delimiter `comment'")
                        ;
                     flag = 1;
                  }
               }
               else
                  error("delimiter `comment' in invalid position");
            }
            /* skip comment sequence including semicolon */
            for (;;)
            {  skip_pad();
               if (line[pos] == 0x1A)
               {  error("comment sequence following `comment' terminate"
                     "d by eof");
                  break;
               }
               if (line[pos++] == ';') break;
            }
            /* get the first basic symbol following comment ... ; */
            scan_symbol();
            /* it may be comment again */
         }
      }
      /* now the current basic symbol is the first symbol of the next
         token */
      token[k].ssn = l_count;
      token[k].code = T_UNDEF;
      token[k].delim = 0;
      token[k].len = 0;
      token[k].image[0] = '\0';
      if (symbol == S_LETTER)
      {  /* letter begins identifier (or letter string) */
         token[k].code = T_IDENT;
         while (symbol == S_LETTER || symbol == S_DIGIT)
            add_char(k, s_char), scan_symbol();
         if (strlen(token[k].image) > T_MAXLEN)
         {  token[k].image[T_MAXLEN] = '\0';
            error("identifier `%s...' too long", token[k].image);
         }
      }
      else if (symbol == S_DIGIT)
      {  /* digit begins numeric constant */
         token[k].code = T_INT;
         /* scan integer part */
         while (symbol == S_DIGIT)
            add_char(k, s_char), scan_symbol();
         /* scan fractional part */
         if (symbol == S_POINT)
         {  token[k].code = T_REAL;
            add_char(k, '.'), scan_symbol();
            /* after point must be digit */
            if (symbol != S_DIGIT)
               error("real constant `%s' incomplete", token[k].image);
frac:       while (symbol == S_DIGIT)
               add_char(k, s_char), scan_symbol();
         }
         /* scan optional decimal exponent part */
         if (symbol == S_TEN)
         {  token[k].code = T_REAL;
            add_char(k, TEN_CHAR), scan_symbol();
dexp:       /* scan optional decimal exponent sign */
            if (symbol == S_PLUS)
               add_char(k, '+'), scan_symbol();
            else if (symbol == S_MINUS)
               add_char(k, '-'), scan_symbol();
            /* after ten symbol or sign must be digit */
            if (symbol != S_DIGIT)
               error("real constant `%s' incomplete", token[k].image);
            while (symbol == S_DIGIT)
               add_char(k, s_char), scan_symbol();
         }
         if (strlen(token[k].image) > T_MAXLEN)
         {  token[k].image[T_MAXLEN] = '\0';
            error("constant `%s...' too long", token[k].image);
         }
      }
      else if (symbol == S_FALSE)
      {  /* logical constant 'false' */
         token[k].code = T_FALSE;
         strcpy(token[k].image, "false"), scan_symbol();
      }
      else if (symbol == S_TRUE)
      {  /* logical constant 'true' */
         token[k].code = T_TRUE;
         strcpy(token[k].image, "true"), scan_symbol();
      }
      else if (symbol == S_OPEN)
      {  /* character string */
         token[k].code = T_STRING;
         add_char(k, '"'); /* opening quote */
         for (;;)
         {  for (;;)
            {  if (line[pos] == 0x1A)
               {  /* eof detected */
                  error("unexpected eof within string");
                  goto clos;
               }
               if (line[pos] == '\0')
               {  error("string incomplete");
                  break;
               }
               if (iscntrl(line[pos]))
               {  error("invalid use of control character 0x%02X within"
                     " string", line[pos]);
                  pos++;
                  continue; /* ignore it */
               }
               if (line[pos] == '\\')
               {  pos++;
                  if (line[pos] == 0x1A || line[pos] == '\0')
                  {  error("invalid use of backslash within string");
                     continue; /* ignore it */
                  }
                  add_char(k, '\\');
               }
               else if (line[pos] == '"')
               {  pos++;
                  break;
               }
               add_char(k, line[pos++]);
            }
            /* string can have another part */
            skip_pad();
            if (line[pos] != '"') break;
            pos++; /* continue to scan another part */
         }
clos:    add_char(k, '"'); /* closing quote */
         scan_symbol(); /* symbol following character string */
      }
      else if (symbol == S_POINT)
      {  /* point begins numeric constant */
         token[k].code = T_REAL;
         add_char(k, '.'), scan_symbol();
         /* after point must be digit */
         if (symbol != S_DIGIT)
         {  error("invalid use of period");
            goto scan; /* ignore it */
         }
         goto frac; /* continue scan numeric constant */
      }
      else if (symbol == S_TEN)
      {  /* ten symbol begins numeric constant */
         token[k].code = T_REAL;
         add_char(k, TEN_CHAR), scan_symbol();
         /* after ten must be sign or digit */
         if (!(symbol == S_PLUS || symbol == S_MINUS ||
               symbol == S_DIGIT))
         {  error("invalid use of subscripted ten");
            goto scan; /* ignore it */
         }
         goto dexp; /* continue scan numeric constant */
      }
      else
      {  /* may be only delimiter */
         char *image;
         token[k].code = T_DELIM;
         token[k].delim = symbol;
         switch (symbol)
         {  case S_EOF:          image = "eof";       break;
            case S_PLUS:         image = "+";         break;
            case S_MINUS:        image = "-";         break;
            case S_TIMES:        image = "*";         break;
            case S_SLASH:        image = "/";         break;
            case S_INTDIV:       image = "%";         break;
            case S_POWER:        image = "^";         break;
            case S_LESS:         image = "<";         break;
            case S_NOTGREATER:   image = "<=";        break;
            case S_EQUAL:        image = "=";         break;
            case S_NOTLESS:      image = ">=";        break;
            case S_GREATER:      image = ">";         break;
            case S_NOTEQUAL:     image = "!=";        break;
            case S_EQUIV:        image = "==";        break;
            case S_IMPL:         image = "->";        break;
            case S_OR:           image = "|";         break;
            case S_AND:          image = "&";         break;
            case S_NOT:          image = "!";         break;
            case S_COMMA:        image = ",";         break;
            case S_COLON:        image = ":";         break;
            case S_SEMICOLON:    image = ";";         break;
            case S_ASSIGN:       image = ":=";        break;
            case S_LEFT:         image = "(";         break;
            case S_RIGHT:        image = ")";         break;
            case S_BEGSUB:       image = "[";         break;
            case S_ENDSUB:       image = "]";         break;
            case S_ARRAY:        image = "array";     break;
            case S_BEGIN:        image = "begin";     break;
            case S_BOOLEAN:      image = "Boolean";   break;
            case S_CODE:         image = "code";      break;
            case S_DO:           image = "do";        break;
            case S_ELSE:         image = "else";      break;
            case S_END:          image = "end";       break;
            case S_FOR:          image = "for";       break;
            case S_GOTO:         image = "go to";     break;
            case S_IF:           image = "if";        break;
            case S_INTEGER:      image = "integer";   break;
            case S_LABEL:        image = "label";     break;
            case S_OWN:          image = "own";       break;
            case S_PROCEDURE:    image = "procedure"; break;
            case S_REAL:         image = "real";      break;
            case S_STEP:         image = "step";      break;
            case S_STRING:       image = "string";    break;
            case S_SWITCH:       image = "switch";    break;
            case S_THEN:         image = "then";      break;
            case S_UNTIL:        image = "until";     break;
            case S_VALUE:        image = "value";     break;
            case S_WHILE:        image = "while";     break;
            default: assert(symbol != symbol);
         }
         strcpy(token[k].image, image), scan_symbol();
      }
      /* it is appropriate place to print the next token */
      if (debug && first_pass)
      {  fprintf(outfile, "%6d: %-6s |%s|\n", token[k].ssn,
            token[k].code == T_IDENT  ? "ident"  :
            token[k].code == T_INT    ? "int"    :
            token[k].code == T_REAL   ? "real"   :
            token[k].code == T_FALSE  ? "false"  :
            token[k].code == T_TRUE   ? "true"   :
            token[k].code == T_STRING ? "string" :
            token[k].code == T_DELIM  ? "delim"  : "???",
            token[k].image);
      }
      return;
};

/*----------------------------------------------------------------------
-- get_token - scanning routine.
--
-- This routine is a scanner that is used by all parsing routines. It
-- copies the current token from token[1] to token[0] and then scans the
-- next token into token[1] or copies token[2] to token[1] (if token[2]
-- is defined). In any case after a call token[2] becomes undefined. */

static void get_token(void)
{     /* token[0] := current token */
      token[0].ssn = token[1].ssn;
      token[0].code = token[1].code;
      token[0].delim = token[1].delim;
      token[0].len = token[1].len;
      strcpy(token[0].image, token[1].image);
      /* token[1] := next token (becomes current) */
      if (token[2].code == T_UNDEF)
         scan_token(1);
      else
      {  token[1].ssn = token[2].ssn;
         token[1].code = token[2].code;
         token[1].delim = token[2].delim;
         token[1].len = token[2].len;
         strcpy(token[1].image, token[2].image);
         /* token[2] is no longer defined */
         token[2].code = T_UNDEF;
      }
      return;
}

/*----------------------------------------------------------------------
-- get_token2 - forescanning routine.
--
-- This routine forescans the token following the current token and
-- assign it to token[2]. The current token remains unchanged. */

static void get_token2(void)
{     if (token[2].code == T_UNDEF) scan_token(2);
      return;
}

/*----------------------------------------------------------------------
-- Two types CODE and CSQE declared below are used to represent output
-- code generated by the translator.
--
-- Any object of CODE type is a character string which is a result of
-- translation of a part of source program corresponding to appropriate
-- non-terminal. For example, if parsing routine has finished parsing
-- of <primary expression>, it returns pointer to object of CODE type
-- that represents appropriate output code written in the C programming
-- language.
--
-- Since several parts of output code need to be essentially rearranged
-- during translation, it is impossible to emit output code immediately
-- to output text file. So we need keep all that parts in the main core
-- as objects of CODE type.
--
-- In order to deal with character strings of arbitrary length each
-- object of CODE type is represented as linked list of objects of CSQE
-- types, where the latter objects represent usual character strings
-- allocated dynamically.
--
-- If an object of CODE type corresponds to expression or to left part
-- of assignment statement some additional semantic information (lvalue
-- flag and type of expression) is linked with such object. This is used
-- further by the other parsing routines to perform appropriate semantic
-- checks and to build proper output code.
--
-- Objects of CODE type actually used only during the second pass. */

typedef struct CODE CODE;
typedef struct CSQE CSQE;

struct CODE
{     /* output code */
      int lval;
      /* lvalue flag (has sense for expression only) */
      int type;
      /* value type (has sense for expression only) */
      CSQE *head;
      /* pointer to the first linked list entry */
      CSQE *tail;
      /* pointer to the last linked list entry */
};

struct CSQE
{     /* output code linked list entry */
      char *str;
      /* pointer to character string */
      CSQE *next;
      /* ponter to next linked list entry */
};

/* These are names of all library routines used in the output code. */
#define a_active_dsa    "active_dsa"
#define a_alloc_array   "alloc_array"
#define a_alloc_same    "alloc_same"
#define a_and           "and"
#define a_copy_bool     "copy_bool"
#define a_copy_int      "copy_int"
#define a_copy_real     "copy_real"
#define a_equal         "equal"
#define a_equiv         "equiv"
#define a_expi          "expi"
#define a_expn          "expn"
#define a_expr          "expr"
#define a_false         "false"
#define a_fault         "fault"
#define a_get_bool      "get_bool"
#define a_get_int       "get_int"
#define a_get_label     "get_label"
#define a_get_real      "get_real"
#define a_global_dsa    "global_dsa"
#define a_go_to         "go_to"
#define a_greater       "greater"
#define a_impl          "impl"
#define a_int2real      "int2real"
#define a_less          "less"
#define a_loc_bool      "loc_bool"
#define a_loc_int       "loc_int"
#define a_loc_real      "loc_real"
#define a_make_arg      "make_arg"
#define a_make_label    "make_label"
#define a_not           "not"
#define a_notequal      "notequal"
#define a_notgreater    "notgreater"
#define a_notless       "notless"
#define a_or            "or"
#define a_own_array     "own_array"
#define a_own_same      "own_same"
#define a_pop_stack     "pop_stack"
#define a_print         "print"
#define a_real2int      "real2int"
#define a_set_bool      "set_bool"
#define a_set_int       "set_int"
#define a_set_real      "set_real"
#define a_stack_top     "stack_top"
#define a_true          "true"

static char codebuf[4000];
/* Auxiliary buffer to format output code. */

/*----------------------------------------------------------------------
-- new_code - create empty output code.
--
-- This routine creates empty output code (i.e. code which is an empty
-- character string) and returns pointer to it. */

static CODE *new_code(void)
{     if (first_pass)
         return NULL;
      else
      {  CODE *code = my_malloc(sizeof(CODE));
         code->lval = code->type = 0;
         code->head = code->tail = NULL;
         return code;
      }
}

/*----------------------------------------------------------------------
-- prepend - prepend character string to output code.
--
-- This routine formats character string (like sprintf) using format
-- fmt and variable parameter list. Then the routine places this string
-- BEFORE character string represented by code. */

static void prepend(CODE *code, char *fmt, ...)
{     /* code := text || code */
      if (second_pass)
      {  va_list arg;
         CSQE *sqe;
         va_start(arg, fmt);
         vsprintf(codebuf, fmt, arg);
         assert(strlen(codebuf) < sizeof(codebuf));
         va_end(arg);
         sqe = my_malloc(sizeof(CSQE));
         sqe->str = my_malloc(strlen(codebuf)+1);
         strcpy(sqe->str, codebuf);
         sqe->next = code->head;
         code->head = sqe;
         if (code->tail == NULL) code->tail = sqe;
      }
      return;
}

/*----------------------------------------------------------------------
-- append - append character string to output code.
--
-- This routine formats character string (like sprintf) using format
-- fmt and variable parameter list. Then the routine places this string
-- AFTER character string represented by code. */

static void append(CODE *code, char *fmt, ...)
{     /* code := code || text */
      if (second_pass)
      {  va_list arg;
         CSQE *sqe;
         va_start(arg, fmt);
         vsprintf(codebuf, fmt, arg);
         assert(strlen(codebuf) < sizeof(codebuf));
         va_end(arg);
         sqe = my_malloc(sizeof(CSQE));
         sqe->str = my_malloc(strlen(codebuf)+1);
         strcpy(sqe->str, codebuf);
         sqe->next = NULL;
         if (code->head == NULL)
            code->head = sqe;
         else
            code->tail->next = sqe;
         code->tail = sqe;
      }
      return;
}

/*----------------------------------------------------------------------
-- catenate - catenate output codes.
--
-- This routine catenates two character strings represented by output
-- code x and y, and assigns resulting string to output code x. After
-- this operation output code y becomes invalid and it can't be used
-- in any further operation. */

static void catenate(CODE *x, CODE *y)
{     /* x := x || y; free(y) */
      if (second_pass)
      {  if (x->head == NULL)
            x->head = y->head;
         else
            x->tail->next = y->head;
         if (y->tail != NULL) x->tail = y->tail;
         my_free(y);
      }
      return;
}

/*----------------------------------------------------------------------
-- free_code - delete output code.
--
-- This routine frees memory used by output code. After this operation
-- pointer to output code becomes invalid. */

static void free_code(CODE *code)
{     if (second_pass)
      {  CSQE *sqe;
         while (code->head != NULL)
         {  sqe = code->head;
            code->head = sqe->next;
            my_free(sqe->str);
            my_free(sqe);
         }
         my_free(code);
      }
      return;
}

/*----------------------------------------------------------------------
-- The type BLOCK declared below is used to represent information about
-- each actual or dummy block that appears in source Algol 60 program.
--
-- The following kinds of block exist:
--
-- dummy block enclosing entire module (the environmental block). This
-- block is the root of the block tree and always has the number 0;
--
-- dummy block representing procedure (procedure block). This block
-- localizes all formal parameters of the corresponding procedure;
--
-- dummy block enclosing procedure body;
--
-- dummy block enclosing statement following 'do';
--
-- ordinary (explicit) program block.
--
-- The type IDENT declared below is used to represent information about
-- each identifier that appears in source Algol 60 program. */

typedef struct BLOCK BLOCK;
typedef struct IDENT IDENT;

struct BLOCK
{     /* program block */
      int seqn;
      /* block sequential number; all blocks are numbered in that order
         in which they appear in source program; the first block (i.e.
         the dummy environmental block enclosing entire module) has the
         number 0; these numbers are used mainly to form unique C names
         for identifiers localized in the corresponding blocks */
      int ssn;
      /* source line number in which block starts */
      IDENT *proc;
      /* pointer to procedure identifier (if this block is dummy block
         representing procedure) or NULL (for blocks of other kinds) */
      IDENT *first, *last;
      /* pointers to first and to last identifiers localized in block */
      BLOCK *surr;
      /* pointer to surround block (to parent block in block tree) */
      BLOCK *next;
      /* pointer to the next block (to block having the next sequential
         number) */
};

static BLOCK *first_b, *last_b;
/* Pointers to first and to last blocks. During the first pass as well
   as the second pass last_b always points to last considered block. */

static BLOCK *current = NULL;
/* Pointer to the current active block. */

struct IDENT
{     /* identifier */
      char *name;
      /* name (image) of identifier */
      int ssn_decl;
      /* source line number where declaration or specification of
         identifier appeared (0 if id is not declared/specified yet) */
      int ssn_used;
      /* source line number where identifier was referenced for the
         first time (0 if id is not referenced yet) */
      int flags;
      /* identifier properties (if flags is zero then identifier is not
         declared/specified yet): */
#define F_REAL    0x0001   /* real */
#define F_INT     0x0002   /* integer */
#define F_BOOL    0x0004   /* Boolean */
#define F_LABEL   0x0008   /* label */
#define F_ARRAY   0x0010   /* array */
#define F_SWITCH  0x0020   /* switch */
#define F_PROC    0x0040   /* procedure */
#define F_STRING  0x0080   /* string */
#define F_BYVAL   0x0100   /* formal parameter called by value */
#define F_BYNAME  0x0200   /* formal parameter called by name */
#define F_OWN     0x0400   /* own */
#define F_CODE    0x0800   /* code procedure */
#define F_BLTIN   0x1000   /* built-in procedure */
      int dim;
      /* identifier dimension (number of subscripts for array, number
         of parameters for procedure, number of label used in longjmp,
         etc.); if dimension is not applicable or is unknown yet then
         dim is equal to -1 */
      BLOCK *block;
      /* pointer to block where identifier is localized */
      IDENT *next;
      /* pointer to the next identifier localized in the same block */
};

/*----------------------------------------------------------------------
-- look_up - search for identifier.
--
-- This routine is used to search for identifier by its name.
--
-- If decl is set then the routine was called to declare identifier,
-- and if decl is clear then the routine was called to make reference
-- to identifier. In any case ssn is the number of source line where
-- the identifier appeared.
--
-- On the first pass the routine performs search only in the current
-- block and if the identifier is not found the routine adds it to the
-- current block (since in Algol 60 declaration of identifier is not
-- necessarily preceding its usage). Of course, an attempt to declare
-- identifier which is already declared in the same block raises error.
--
-- On the second pass all identifiers are localized in blocks properly
-- and there are no undeclared identifiers. So the routine performs
-- a search in blocks of all levels starting from the current block. */

static int array_decl_flag = 0;
/* This flag is set when bound expression is processed. This is used
   to catch invalid references to identifiers that are declared in the
   same block as array */

static IDENT *look_up(char *name, int decl, int ssn)
{     IDENT *id;
      if (first_pass)
      {  /* search only in the current block */
         for (id = current->first; id != NULL; id = id->next)
            if (strcmp(id->name, name) == 0) break;
         /* check for multiple declaration */
         if (decl && id != NULL && id->flags != 0)
         {  error("identifier `%s' multiply declared (see line %d)",
               id->name, id->ssn_decl);
            id = NULL; /* as if it were not found */
         }
         /* if identifier not found then add it to the current block */
         if (id == NULL)
         {  id = my_malloc(sizeof(IDENT));
            id->name = my_malloc(strlen(name)+1);
            strcpy(id->name, name);
            id->ssn_decl = id->ssn_used = 0;
            id->flags = 0;
            id->dim = -1;
            id->block = current;
            id->next = NULL;
            if (current->first == NULL)
               current->first = id;
            else
               current->last->next = id;
            current->last = id;
         }
         /* set source line number */
         if (decl)
            id->ssn_decl = ssn;
         else
            if (id->ssn_used == 0) id->ssn_used = ssn;
      }
      else
      {  /* search in blocks of all levels starting from the current
            block */
         BLOCK *b;
         for (b = current; ; b = b->surr)
         {  assert(b != NULL);
            for (id = b->first; id != NULL; id = id->next)
               if (strcmp(id->name, name) == 0) break;
            if (id != NULL) break;
         }
         /* check clause 5.2.4.2 (see Modified Report) */
         if (array_decl_flag && !decl && id->block == current)
            error("identifier `%s' in bound expression declared in same"
            " program block as array", id->name);
      }
      return id;
}

/*----------------------------------------------------------------------
-- to_real - generate code to convert integer expression to real one.
--
-- This routine generates code to convert expression of integer type to
-- expression of real type. */

static void to_real(CODE *x)
{     if (second_pass && x->type == F_INT)
      {  x->lval = 0;
         x->type = F_REAL;
         prepend(x, a_int2real "(");
         append(x, ")");
      }
      return;
}

/*----------------------------------------------------------------------
-- to_int - generate code to convert real expression to integer one.
--
-- This routine generates code to convert expression of real type to
-- expression of integer type. */

static void to_int(CODE *x)
{     if (second_pass && x->type == F_REAL)
      {  x->lval = 0;
         x->type = F_INT;
         prepend(x, a_real2int "(");
         append(x, ")");
      }
      return;
}

/*----------------------------------------------------------------------
-- dsa_level - determine DSA level.
--
-- This routine determines DSA (Dynamic Storage Area) level of those
-- procedure in some block of which identifier id is declared.
--
-- DSA level is the level of procedure in the procedure tree. Outermost
-- procedure (i.e. precompiled procedure or dummy procedure representing
-- main program) has level 0. This level is used to access to that DSA
-- where the appropriate identifier is placed. */

static int dsa_level(IDENT *id)
{     BLOCK *b;
      int level = -1;
      for (b = id->block; b != NULL; b = b->surr)
         if (b->proc != NULL) level++;
      return level;
}

/*----------------------------------------------------------------------
-- current_level - determine current procedure level.
--
-- This routine determines level of current procedure in the same way
-- as the routine dsa_level (see above). */

static int current_level(void)
{     BLOCK *b;
      int level = -1;
      for (b = current; b != NULL; b = b->surr)
         if (b->proc != NULL) level++;
      return level;
}

/*----------------------------------------------------------------------
-- subscripted_variable - parse subscripted variable.
--
-- This routine parses subscripted variable using the syntax:
--
-- <subscripted variable> ::= <identifier> [ <subscript list> ]
-- <subscript list> ::= <expression>
-- <subscript list> ::= <subscript list> , <expression>
--
-- Output code generated by the routine has the form:
--
-- (*loc_xxx(dv, n, i1, i2, ..., in))
--
-- where loc_xxx is library routine loc_real, loc_int, or loc_bool that
-- computes pointer to appropriate array element (subscripted variable);
-- dv is dope vector of array placed in DSA (for local arrays) or in
-- static storage (for own arrays), n is number of subscripts, and i1,
-- i2, ..., in are subscript expressions. */

static CODE *expression(void);

static CODE *subscripted_variable(void)
{     CODE *code = new_code();
      IDENT *arr; /* array identifier */
      int dim = 0; /* number of subscripts */
      char *place;
      /* the current token must be (array) identifier */
      assert(t_code == T_IDENT);
      arr = look_up(t_image, 0, t_ssn);
      if (second_pass && !(arr->flags & F_ARRAY))
         error("invalid use of `%s' as array identifier", arr->name);
      /* generate head of code */
      if (second_pass)
      {  code->lval = 1;
         code->type = arr->flags & (F_REAL | F_INT | F_BOOL);
         append(code, "(*%s(",
            code->type == F_REAL ? a_loc_real :
            code->type == F_INT  ? a_loc_int  : a_loc_bool);
         if (arr->flags & F_OWN)
         {  /* own array */
            append(code, "%s_%d, ?, ",
               arr->name, arr->block->seqn);
         }
         else
         {  /* local or formal array */
            append(code, "dsa_%d->%s_%d, ?, ",
               dsa_level(arr), arr->name, arr->block->seqn);
         }
         /* remember place to substitute number of subscripts */
         place = strchr(code->tail->str, '?');
         assert(place);
      }
      get_token(/* id */);
      /* after (array) identifier there must be '[' */
      assert(t_delim(S_BEGSUB));
      /* process subscript list */
      for (;;)
      {  /* parse the next subscript expression */
         CODE *expr;
         if (dim == 9)
         {  error("number of subscripts exceeds allowable maximum");
            dim = 0;
         }
         get_token(/* [ or , */);
         expr = expression(), to_int(expr);
         if (second_pass && expr->type != F_INT)
         {  error("invalid type of subscript expression");
            expr->type = F_INT;
         }
         catenate(code, expr);
         dim++;
         if (!t_delim(S_COMMA)) break;
         append(code, ", ");
      }
      if (!t_delim(S_ENDSUB))
         error("missing right parenthesis in subscripted variable");
      /* check number of subscripts */
      if (arr->dim < 0) arr->dim = dim;
      if (second_pass && (arr->flags & F_ARRAY) && arr->dim != dim)
      {  if (arr->flags & (F_BYVAL | F_BYNAME))
            error("number of subscripts in subscripted variable conflic"
               "ts with earlier use of array `%s'", arr->name);
         else
            error("number of subscripts in subscripted variable conflic"
               "ts with declaration of array `%s' at line %d",
               arr->name, arr->ssn_decl);
      }
      if (t_delim(S_ENDSUB)) get_token(/* ] */);
      /* generate tail of code */
      assert(1 <= dim && dim <= 9);
      if (second_pass) *place = (char)(dim + '0');
      append(code, "))");
      return code;
}

/*----------------------------------------------------------------------
-- switch_designator - parse switch designator.
--
-- This routine parses switch designator using the syntax:
--
-- <switch designator> ::= <identifier> [ <expression> ]
--
-- Output code generated by the routine has the form:
--
-- (global_dsa = ..., id(k))
--
-- where id is name of routine that computes value of designational
-- expression (value of label type represented by struct label), k is
-- subscript expression.
--
-- If switch is local then global_dsa is assigned to point to dsa of
-- calling procedure, and id is name of static routine generated from
-- switch declaration. If switch is formal then global_dsa is assigned
-- to arg.arg2 and id is arg.arg1, where arg is formal parameter that
-- represents formal switch. */

static CODE *switch_designator(void)
{     CODE *code;
      IDENT *swit; /* switch identifier */
      int dim = 0; /* number of subscripts */
      assert(second_pass);
      /* the current token must be switch identifier */
      assert(t_code == T_IDENT);
      swit = look_up(t_image, 0, t_ssn);
      assert(swit->flags & F_SWITCH);
      /* although switch designator has only one subscript, we should
         process it as it were subscripted variable, since on the first
         pass switch designator is processed by subscripted_variable
         routine */
      get_token(/* id */);
      /* after switch identifier there must be '[' */
      assert(t_delim(S_BEGSUB));
      /* process subscript list */
      for (;;)
      {  /* parse the next subscript expression */
         if (dim == 1)
            error("invalid number of subscripts in switch designator fo"
               "r `%s'", swit->name);
         get_token(/* [ or , */);
         code = expression(), to_int(code);
         if (code->type != F_INT)
         {  error("invalid type of subscript expression");
            code->type = F_INT;
         }
         dim++;
         if (!t_delim(S_COMMA)) break;
      }
      /* after subscript list there must be ']' */
      assert(t_delim(S_ENDSUB));
      get_token(/* ] */);
      /* generate code */
      code->lval = 0;
      code->type = F_LABEL;
      if (swit->flags & F_BYNAME)
      {  /* formal switch */
         prepend(code, "(" a_global_dsa " = dsa_%d->%s_%d.arg2, (*(stru"
            "ct label (*)(int))dsa_%d->%s_%d.arg1)(",
            dsa_level(swit), swit->name, swit->block->seqn,
            dsa_level(swit), swit->name, swit->block->seqn);
      }
      else
      {  /* local switch */
         prepend(code, "(" a_global_dsa " = (void *)dsa_%d, %s_%d(",
            current_level(), swit->name, swit->block->seqn);
      }
      append(code, "))");
      return code;
}

/*----------------------------------------------------------------------
-- emit_dsa_pointers - generate code for DSA pointers.
--
-- This routine generates code to initialize DSA pointers. These
-- pointers are used in each routine representing thunk, or switch, or
-- statement following 'do' to access to identifiers placed in DSAs.
-- It is assumed that initialization is performed after entering to the
-- corresponding routine and global_dsa points to DSA of the current
-- procedure in some block of which the corresponding actual parameter,
-- or switch declaration, or statement following 'do' are placed.
--
-- Generated code is immediately appended to final output code. */

static CODE *emit = NULL;
/* Final output code. This code is the accumulated result of translation
   and corresponds to the root of parsing tree. */

static void emit_dsa_pointers(void)
{     BLOCK *b;
      int level = current_level(); /* level of current procedure */
      for (b = current; b != NULL; b = b->surr)
      {  if (b->proc == NULL) continue;
            append(emit, "      register struct dsa_%s_%d *dsa_%d = (vo"
               "id *)" a_global_dsa "->vector[%d];\n",
               b->proc->name, b->proc->block->seqn, level, level);
         level--;
      }
      return;
}

/*----------------------------------------------------------------------
-- actual_parameter - parse actual parameter.
--
-- This routine parses the next actual parameter using the syntax:
--
-- <actual parameter> ::= <constant>
-- <actual parameter> ::= <identifier>
-- <actual parameter> ::= <expression>
-- <actual parameter> ::= <string>
--
-- If arg != NULL then it points to an identifier of appropriate formal
-- parameter. This is used to check actual-formal correspondence.
--
-- Generated output code always has the form:
--
-- make_arg(arg1, arg2)
--
-- where make_arg is library routine that makes struct arg representing
-- actual parameter of any kind using two pointers arg1 and arg2.
--
-- If actual parameter is quoted string then arg1 points to the string
-- body (i.e. to the first character), and arg2 is NULL.
--
-- If actual parameter is array identifier then arg1 points to the dope
-- vector of the array, and arg2 is 'r', 'i', or 'b' (casted to void *)
-- to determine array type (real, integer, or Boolean).
--
-- If actual parameter is formal parameter called by name (but not an
-- array) then make_arg routine is not used since pair (arg1, arg2) as
-- struct arg is placed in DSA of the appropriate procedure.
--
-- If actual parameter is expression (including simple or subscripted
-- variable) then it is translated to static routine (so called thunk).
-- That routine computes struct desc that represents value of expression
-- (including value of label), or pointer to variable. In this case
-- arg1 is pointer to thunk, and arg2 is pointer to DSA of calling
-- procedure (used by thunk to access to appropriate identifiers).
--
-- If actual parameter is switch or procedure identifier then arg1 is
-- pointer to routine representing switch or procedure declaration, and
-- arg2 is again pointer to DSA of calling procedure. */

static int thunk_count = 0;
/* Thunk count. This value is used to form unique name of corresponding
   static routine. */

static int thunk_real0 = 0; /* thunk count for 0.0 */
static int thunk_real1 = 0; /* thunk count for 1.0 */
static int thunk_int0  = 0; /* thunk count for 0 */
static int thunk_int1  = 0; /* thunk count for 1 */
static int thunk_false = 0; /* thunk count for false */
static int thunk_true  = 0; /* thunk count for true */

static CODE *emit_ssn(int ssn);

static CODE *actual_parameter(IDENT *arg)
{     CODE *code = new_code();
      /* the current token is a head of actual parameter */
      if (t_code == T_STRING)
      {  /* the corresponding formal parameter must be string only */
         if (second_pass && arg != NULL && !(arg->flags & F_STRING))
         {  error("string passed as actual parameter conflicts with kin"
               "d of formal parameter `%s' as specified in declaration "
               "of procedure `%s' beginning at line %d", arg->name,
               arg->block->proc->name, arg->block->proc->ssn_decl);
            goto skip1;
         }
         /* generate code for actual string */
         append(code, a_make_arg "(");
         append(code, t_image);
         append(code, ", NULL)");
skip1:   get_token(/* string */);
         goto done;
      }
      /* to make parsing easier we need hint token[2] */
      get_token2();
      /* special cases when actual parameter is identifier */
      if (t_code == T_IDENT && token[2].code == T_DELIM &&
         (token[2].delim == S_COMMA || token[2].delim == S_RIGHT))
      {  IDENT *id = look_up(t_image, 0, t_ssn);
         /* at the first pass any identifier being actual parameter is
            treated as expression */
         if (second_pass)
         {  if (id->flags == (F_REAL | F_BYNAME) ||
                id->flags == (F_INT  | F_BYNAME) ||
                id->flags == (F_BOOL | F_BYNAME))
            {  /* actual parameter is a simple formal parameter called
                  by name (except labels) */
               if (arg != NULL)
               {  /* the corresponding formal parameter must be simple
                     formal parameter of appropriate type */
                  int actual_type, formal_type;
                  if (arg->flags & ~(F_REAL | F_INT | F_BOOL | F_BYVAL |
                     F_BYNAME))
                  {  error("formal parameter `%s' called by name and pa"
                        "ssed as actual parameter conflicts with kind o"
                        "f formal parameter `%s' as specified in declar"
                        "ation of procedure `%s' beginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip1a;
                  }
                  actual_type = id->flags  & (F_REAL | F_INT | F_BOOL);
                  formal_type = arg->flags & (F_REAL | F_INT | F_BOOL);
                  if (actual_type & (F_REAL | F_INT))
                  {  actual_type &= ~(F_REAL | F_INT);
                     formal_type &= ~(F_REAL | F_INT);
                  }
                  if (actual_type != formal_type)
                  {  error("type of formal parameter `%s' called by nam"
                        "e and passed as actual parameter conflicts wit"
                        "h type of formal parameter `%s' as specified i"
                        "n declaration of procedure `%s' beginning at l"
                        "ine %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip1a;
                  }
               }
               /* generate code */
               append(code, "dsa_%d->%s_%d", dsa_level(id), id->name,
                  id->block->seqn);
skip1a:        get_token(/* id */);
               goto done;
            }
            if (id->flags & F_ARRAY)
            {  /* actual parameter is array identifier */
               if (arg != NULL)
               {  /* so the corresponding formal parameter must be only
                     array of appropriate type and dimension */
                  int actual_type, formal_type;
                  if (!(arg->flags & F_ARRAY))
                  {  error("array `%s' passed as actual parameter confl"
                        "icts with kind of formal parameter `%s' as spe"
                        "cified in declaration of procedure `%s' beginn"
                        "ing at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip2;
                  }
                  actual_type = id->flags  & (F_REAL | F_INT | F_BOOL);
                  formal_type = arg->flags & (F_REAL | F_INT | F_BOOL);
                  if (arg->flags & F_BYVAL)
                  {  actual_type &= ~(F_REAL | F_INT);
                     formal_type &= ~(F_REAL | F_INT);
                  }
                  if (actual_type != formal_type)
                  {  error("type of array `%s' passed as actual paramet"
                        "er conflicts with type of formal array `%s' as"
                        " specified in declaration of procedure `%s' be"
                        "ginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip2;
                  }
                  if (id->dim >= 0 && arg->dim >= 0 &&
                      id->dim != arg->dim)
                  {  error("dimension of array `%s' passed as actual pa"
                        "rameter not equal to dimension of formal array"
                        " `%s' as implied in declaration of procedure `"
                        "%s' beginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip2;
                  }
               }
               /* generate code */
               if (!(id->flags & F_OWN))
               {  /* local or formal array */
                  append(code, a_make_arg "(dsa_%d->%s_%d",
                     dsa_level(id), id->name, id->block->seqn);
               }
               else
               {  /* own array */
                  append(code, a_make_arg "(%s_%d",
                     id->name, id->block->seqn);
               }
               append(code, ", (void *)'%c')",
                  (id->flags & F_REAL) ? 'r' :
                  (id->flags & F_INT)  ? 'i' :
                  (id->flags & F_BOOL) ? 'b' : '?');
skip2:         get_token(/* id */);
               goto done;
            }
            if (id->flags & F_SWITCH)
            {  /* actual parameter is switch identifier */
               if (arg != NULL)
               {  /* so the corresponding formal parameter must be only
                     switch */
                  if (!(arg->flags & F_SWITCH))
                  {  error("switch `%s' passed as actual parameter conf"
                        "licts with kind of formal parameter `%s' as sp"
                        "ecified in declaration of procedure `%s' begin"
                        "ning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip3;
                  }
               }
               /* generate code */
               if (!(id->flags & F_BYNAME))
               {  /* local switch */
                  append(code, a_make_arg "((void *)%s_%d, dsa_%d)",
                     id->name, id->block->seqn, current_level());
               }
               else
               {  /* formal switch */
                  append(code, "dsa_%d->%s_%d",
                     dsa_level(id), id->name, id->block->seqn);
               }
skip3:         get_token(/* id */);
               goto done;
            }
            if (id->flags & F_PROC)
            {  /* actual parameter is procedure identifier */
               if (arg != NULL)
               {  /* so the corresponding formal parameter must be only
                     formal procedure of appropriate type and dimension,
                     or simple formal parameter of appropriate type
                     (since identifier of <type> procedure having no
                     parameters is in itself an expression) */
                  int simple = !(arg->flags & ~(F_REAL | F_INT | F_BOOL
                     | F_BYVAL | F_BYNAME));
                  int actual_type, formal_type;
                  if (!(simple || (arg->flags & F_PROC)))
                  {  error("procedure `%s' passed as actual parameter c"
                        "onflicts with kind of formal parameter `%s' as"
                        " specified in declaration of procedure `%s' be"
                        "ginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip4;
                  }
                  actual_type = id->flags  & (F_REAL | F_INT | F_BOOL);
                  formal_type = arg->flags & (F_REAL | F_INT | F_BOOL);
                  if (actual_type & (F_REAL | F_INT))
                  {  actual_type &= ~(F_REAL | F_INT);
                     formal_type &= ~(F_REAL | F_INT);
                  }
                  if (!simple) goto proc;
                  /* if the corresponding formal parameter is simple
                     formal parameter then actual procedure must have
                     appropriate type and empty formal parameter part */
                  if (!(id->flags & (F_REAL | F_INT | F_BOOL)) ||
                        id->dim > 0)
                  {  error("procedure identifier `%s' that is not in it"
                        "self a complete expression and passed as actua"
                        "l parameter conflicts with kind of formal para"
                        "meter `%s' as specified in declaration of proc"
                        "edure `%s' beginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip4;
                  }
                  if (actual_type != formal_type)
                  {  error("procedure identifier `%s' that is in itself"
                        " a complete expression and passed as actual pa"
                        "rameter conflicts with type of formal paramete"
                        "r `%s' as specified in declaration of procedur"
                        "e `%s' beginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip4;
                  }
                  goto gen1;
proc:             /* the corresponding formal parameter is formal
                     procedure that must be of appropriate type and
                     dimension (deeper checking is more expensive and
                     sometimes impossible, that's why omitted) */
                  if (actual_type != formal_type && formal_type)
                  {  error("type of procedure `%s' passed as actual par"
                        "ameter conflicts with type of formal procedure"
                        " `%s' as specified in declaration of procedure"
                        " `%s' beginning at line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip4;
                  }
                  if (id->dim >= 0 && arg->dim >= 0 &&
                      id->dim != arg->dim)
                  {  error("number of parameters of procedure `%s' pass"
                        "ed as actual parameter not equal to number of "
                        "parameters of formal procedure `%s' as implied"
                        " in declaration of procedure `%s' beginning at"
                        " line %d",
                        id->name, arg->name, arg->block->proc->name,
                        arg->block->proc->ssn_decl);
                     goto skip4;
                  }
               }
gen1:          /* generate code */
               if (!(id->flags & F_BYNAME))
               {  /* local procedure */
                  append(code, a_make_arg "((void *)%s_%d, dsa_%d)",
                     id->name, id->block->seqn, current_level());
               }
               else
               {  /* formal procedure */
                  append(code, "dsa_%d->%s_%d",
                     dsa_level(id), id->name, id->block->seqn);
               }
skip4:         get_token(/* id */);
               goto done;
            }
            if (id->flags & F_STRING)
            {  /* actual parameter is formal string identifier */
               if (arg != NULL && !(arg->flags & F_STRING))
               {  error("formal string passed as actual parameter confl"
                     "icts with kind of formal parameter `%s' as specif"
                     "ied in declaration of procedure `%s' beginning at"
                     " line %d", arg->name, arg->block->proc->name,
                     arg->block->proc->ssn_decl);
                  goto skip5;
               }
               append(code, a_make_arg "(dsa_%d->%s_%d, NULL)",
                  dsa_level(id), id->name, id->block->seqn);
skip5:         get_token(/* id */);
               goto done;
            }
            /* in the other cases identifier as actual parameter is
               treated as expression */
         }
      }
      /* actual parameter is expression */
      {  CODE *expr;
         int need_dsa = 1, thunk, ssn;
         /* check for constant */
         if ((t_code == T_REAL || t_code == T_INT ||
              t_code == T_FALSE || t_code == T_TRUE) &&
              token[2].code == T_DELIM &&
             (token[2].delim == S_COMMA || token[2].delim == S_RIGHT))
         {  /* if expression is constant then dsa pointers is not
               needed */
            /* for often used constants thunks generated only once */
            need_dsa = 0;
            if (second_pass && t_code == T_REAL)
            {  if (strcmp(t_image, "0.0") == 0 ||
                   strcmp(t_image,  ".0") == 0)
               {  /* real constant 0.0 */
                  if (thunk_real0 != 0)
                  {  thunk = thunk_real0, get_token(/* 0.0 */);
                     goto gen2;
                  }
                  else
                     thunk_real0 = thunk_count + 1;
               }
               else if (strcmp(t_image, "1.0") == 0)
               {  /* real constant 1.0 */
                  if (thunk_real1 != 0)
                  {  thunk = thunk_real1, get_token(/* 1.0 */);
                     goto gen2;
                  }
                  else
                     thunk_real1 = thunk_count + 1;
               }
            }
            else if (second_pass && t_code == T_INT)
            {  if (strcmp(t_image, "0") == 0)
               {  /* integer constant 0 */
                  if (thunk_int0 != 0)
                  {  thunk = thunk_int0, get_token(/* 0 */);
                     goto gen2;
                  }
                  else
                     thunk_int0 = thunk_count + 1;
               }
               else if (strcmp(t_image, "1") == 0)
               {  /* integer constant 1 */
                  if (thunk_int1 != 0)
                  {  thunk = thunk_int1, get_token(/* 1 */);
                     goto gen2;
                  }
                  else
                     thunk_int1 = thunk_count + 1;
               }
            }
            else if (second_pass && t_code == T_FALSE)
            {  /* Boolean constant false */
               if (thunk_false != 0)
               {  thunk = thunk_false, get_token(/* false */);
                  goto gen2;
               }
               else
                  thunk_false = thunk_count + 1;
            }
            else if (second_pass && t_code == T_TRUE)
            {  /* Boolean constant true */
               if (thunk_true != 0)
               {  thunk = thunk_true, get_token(/* true */);
                  goto gen2;
               }
               else
                  thunk_true = thunk_count + 1;
            }
         }
         /* general expression which is actual parameter */
         ssn = t_ssn;
         expr = expression();
         if (second_pass)
         {  if (arg != NULL)
            {  /* the corresponding formal parameter must be simple
                  formal parameter of appropriate type */
               int actual_type, formal_type;
               if (arg->flags & ~(F_REAL | F_INT | F_BOOL | F_LABEL |
                     F_BYVAL | F_BYNAME))
               {  error("expression passed as actual parameter conflict"
                     "s with kind of formal parameter `%s' as specified"
                     " in declaration of procedure `%s' beginning at li"
                     "ne %d", arg->name, arg->block->proc->name,
                     arg->block->proc->ssn_decl);
                  goto skip6;
               }
               actual_type = expr->type;
               formal_type = arg->flags &
                  (F_REAL | F_INT | F_BOOL | F_LABEL);
               if (actual_type & (F_REAL | F_INT))
               {  actual_type &= ~(F_REAL | F_INT);
                  formal_type &= ~(F_REAL | F_INT);
               }
               if (actual_type != formal_type)
               {  error("type of expression passed as actual parameter "
                     "conflicts with type of formal parameter `%s' as s"
                     "pecified in declaration of procedure `%s' beginni"
                     "ng at line %d", arg->name, arg->block->proc->name,
                     arg->block->proc->ssn_decl);
                  goto skip6;
               }
            }
            /* generate thunk to evaluate expression */
            thunk_count++;
            append(emit, "static struct desc _thunk_%d(void)\n",
               thunk_count);
            append(emit, "{     /* actual parameter at line %d */\n",
               ssn);
            append(emit, "      struct desc res;\n");
            if (need_dsa)
            {  emit_dsa_pointers();
               catenate(emit, emit_ssn(ssn));
            }
            append(emit, "      res.lval = %d;\n", expr->lval);
            switch (expr->type)
            {  case F_REAL:
                  append(emit, "      res.type = 'r';\n");
                  if (expr->lval)
                     append(emit, "      res.u.real_ptr = ");
                  else
                     append(emit, "      res.u.real_val = ");
                  break;
               case F_INT:
                  append(emit, "      res.type = 'i';\n");
                  if (expr->lval)
                     append(emit, "      res.u.int_ptr = ");
                  else
                     append(emit, "      res.u.int_val = ");
                  break;
               case F_BOOL:
                  append(emit, "      res.type = 'b';\n");
                  if (expr->lval)
                     append(emit, "      res.u.bool_ptr = ");
                  else
                     append(emit, "      res.u.bool_val = ");
                  break;
               case F_LABEL:
                  append(emit, "      res.type = 'l';\n");
                  append(emit, "      res.u.label = ");
                  break;
               default:
                  assert(expr->type != expr->type);
            }
            if (expr->lval)
            {  append(emit, "&(");
               catenate(emit, expr);
               append(emit, ")");
            }
            else
               catenate(emit, expr);
            append(emit, ";\n");
            append(emit, "      return res;\n");
            append(emit, "}\n");
            append(emit, "\n");
            /* generate code for actual parameter */
            thunk = thunk_count;
gen2:       append(code, a_make_arg "((void *)_thunk_%d, dsa_%d)",
               thunk, current_level());
skip6:      ;
         }
      }
done: return code;
}

/*----------------------------------------------------------------------
-- ext_comma - parse extended parameter delimiter.
--
-- This routine parses extended parameter delimiter using the following
-- syntax:
--
-- <parameter delimiter> ::= ,
-- <parameter delimiter> ::= ) <letter string> : (
-- <letter string> ::= <letter>
-- <letter string> ::= <letter string> <letter>
--
-- If the current context is an extended parameter delimiter, then the
-- routine returns non-zero, otherwise zero. */

static int ext_comma(void)
{     if (t_delim(S_COMMA))
      {  get_token(/* , */);
         return 1;
      }
      if (t_delim(S_RIGHT))
      {  get_token2();
         if (token[2].code != T_IDENT) return 0;
         /* it is a parameter delimiter */
         get_token(/* ) */);
         assert(t_code == T_IDENT);
         /* check that identifier is a letter string */
         {  char *t;
            for (t = t_image; *t; t++)
            {  if (!isalpha(*t))
               {  error("invalid letter string in parameter delimiter");
                  break;
               }
            }
         }
         get_token(/* letter string */);
         if (t_delim(S_COLON))
            get_token(/* : */);
         else
            error("missing colon in parameter delimiter");
         if (t_delim(S_LEFT))
            get_token(/* ( */);
         else
            error("missing left parenthesis in parameter delimiter");
         return 1;
      }
      return 0;
}

/*----------------------------------------------------------------------
-- function_designator - parse function designator.
--
-- This routine parses function designator (if stmt is clear) or
-- procedure statement (if stmt is set) using the syntax:
--
-- <function designator> ::= <identifier>
-- <function designator> ::= <identifier> ( <actual list> )
-- <actual list> ::= <actual parameter>
-- <actual list> ::= <actual list> , <actual parameter>
--
-- Note that <function designator> is syntactically equivalent to
-- <procedure statement>.
--
-- Output code generated by the routine has the form:
--
-- get_xxx((global_dsa = ..., id(p1, p2, ..., pn)))
--
-- where get_xxx is library routine get_real, get_int, or get_bool,
-- that converts struct desc to real, int, or bool respectively; id is
-- name of routine that represents procedure declaration and computes
-- value in form of struct desc; p1, p2, ..., pn are actual parameters
-- in form of struct arg.
--
-- If called procedure is local then global_dsa is assigned to point to
-- dsa of calling procedure, and id is name of static routine generated
-- from procedure declaration. If called procedure is formal one, then
-- global_dsa is assigned to arg.arg2 and id is arg.arg1, where arg is
-- formal parameter that represents formal procedure. */

static CODE *function_designator(int stmt)
{     IDENT *proc; /* procedure identifier */
      IDENT *arg = NULL; /* formal parameter identifier */
      CODE *code = new_code();
      int list; /* if there is parameter list */
      int dim = 0; /* number of actual parameters */
      /* the current token must be (procedure) identifier */
      assert(t_code == T_IDENT);
      proc = look_up(t_image, 0, t_ssn);
      if (second_pass)
      {  /* identifier must be only procedure identifier */
         if (!(proc->flags & F_PROC))
            error("invalid use of `%s' as procedure identifier",
               proc->name);
         code->lval = 0;
         code->type = proc->flags & (F_REAL | F_INT | F_BOOL);
         if ((proc->flags & F_BLTIN) &&
             (strcmp(proc->name, "inline") == 0 ||
              strcmp(proc->name, "print") == 0))
         {  /* use of pseudo procedures inline and print in function
               designator is not allowed */
            error("invalid use of pseudo procedure `%s' in function des"
               "ignator", proc->name);
         }
         else if ((proc->flags & F_PROC) && code->type == 0 && !stmt)
         {  /* use of typeless procedure is allowed only in procedure
               statement, but not in function designator */
            error("invalid use of typeless procedure `%s' in function d"
               "esignator", proc->name);
         }
         /* generate the head of code */
         append(code, "%s(",
            (code->type & F_REAL) ? a_get_real :
            (code->type & F_INT)  ? a_get_int  :
            (code->type & F_BOOL) ? a_get_bool : "" /* void */);
         if (proc->flags & F_BYNAME)
         {  /* formal procedure */
            append(code, "(" a_global_dsa " = dsa_%d->%s_%d.arg2, (*(st"
               "ruct desc (*)())dsa_%d->%s_%d.arg1)(",
               dsa_level(proc), proc->name, proc->block->seqn,
               dsa_level(proc), proc->name, proc->block->seqn);
         }
         else
         {  /* local procedure */
            append(code, "(" a_global_dsa " = (void *)dsa_%d, %s_%d(",
               current_level(), proc->name, proc->block->seqn);
         }
      }
      get_token(/* id */);
      /* process actual parameter part */
      list = t_delim(S_LEFT);
      if (!list) goto skip; /* actual parameter part is empty */
      /* get pointer to the first formal parameter in order to check
         actual-formal correspondence (this checking is performed on
         the second pass and is possible only for local procedure) */
      if (second_pass && (proc->flags & F_PROC) &&
         !(proc->flags & F_BYNAME))
      {  /* all formal parameters are localized in procedure block */
         BLOCK *b;
         for (b = first_b; b != NULL; b = b->next)
            if (b->proc == proc) break;
         assert(b != NULL);
         arg = b->first;
      }
      /* process actual parameter list */
      get_token(/* ( */);
      for (;;)
      {  /* translate the current actual parameter */
         catenate(code, actual_parameter(arg));
         dim++;
         if (!ext_comma()) break;
         append(code, ", ");
         /* get pointer to the next formal parameter */
         if (second_pass && arg != NULL) arg = arg->next;
         /* extended parameter delimiter following actual parameter
            means that more actual parameter is expected */
      }
      /* end of actual parameter list */
      if (!t_delim(S_RIGHT))
         error("missing right parenthesis after actual parameter list");
skip: /* check number of actual parameter */
      if (proc->dim < 0) proc->dim = dim;
      if (second_pass && (proc->flags & F_PROC) && proc->dim != dim)
      {  if (proc->flags & F_BYNAME)
            error("number of parameters in function designator or proce"
               "dure statement conflicts with earlier use of procedure "
               "`%s'", proc->name);
         else
            error("number of parameters in function designator or proce"
               "dure statement conflicts with declaration of procedure "
               "`%s' beginning at line %d", proc->name, proc->ssn_decl);
      }
      if (list && t_delim(S_RIGHT)) get_token(/* ) */);
      /* generate the tail of code */
      append(code, ")))");
      return code;
}

/*----------------------------------------------------------------------
-- block_level - determine current block level.
--
-- This routine determines level of current block. This level is depth
-- of block in procedure, i.e. in block subtree the root of which is
-- procedure block that has level 0. */

static int block_level(BLOCK *b)
{     int level = -1;
      for (b = b; b != NULL; b = b->surr)
      {  level++;
         if (b->proc != NULL) break;
      }
      assert(level >= 0);
      return level;
}

/*----------------------------------------------------------------------
-- call_by_name - generate code to call by name.
--
-- This routine generates output code to call formal parameter id by
-- name (the corresponding actual parameter must be thunk or procedure
-- with empty parameter part).
--
-- Output code generated by the routine has the form:
--
-- (global_dsa = arg.arg2, arg.arg1())
--
-- where arg is object of struct arg type representing formal parameter
-- to be called. The result of evaluation if this code is always value
-- represented by struct desc. */

static CODE *call_by_name(IDENT *id)
{     CODE *code = new_code();
      if (second_pass)
      {  append(code,
            "(" a_global_dsa " = dsa_%d->%s_%d.arg2, (*(struct desc (*)"
            "(void))dsa_%d->%s_%d.arg1)())",
             dsa_level(id), id->name, id->block->seqn,
             dsa_level(id), id->name, id->block->seqn);
      }
      return code;
}

/*----------------------------------------------------------------------
-- primary - parse primary expression.
--
-- This routine parse primary expression using the following syntax:
--
-- <primary expression> ::= <constant>
-- <primary expression> ::= <identifier>
-- <primary expression> ::= <subscripted variable>
-- <primary expression> ::= <switch designator>
-- <primary expression> ::= <function designator>
-- <primary expression> ::= ( <expression> )
--
-- If primary expression is a constant then output code has the form:
--
-- con
--
-- where con is appropriate constant valid in the C.
--
-- If primary expression is identifier of local or own simple variable,
-- or is identifier of formal parameter called by value then output code
-- has the form:
--
-- dsa_k->id_n (or id_n for own variables)
--
-- where k is level of DSA in which identifier is placed, and n is the
-- number of block used as suffix.
--
-- If primary expression is identifier of simple formal parameter called
-- by name then output code has the form:
--
-- get_xxx((global_dsa = arg.arg2, arg.arg1()))
--
-- where get_xxx is library routine get_real, get_int, or get_bool,
-- that converts struct desc (returned by thunk or routine corresponding
-- to type procedure with empty formal parameter list) to real, int, or
-- bool respectively (see also the routine call_by_name).
--
-- If primary expression is identifier of local label then output code
-- has the form:
--
-- make_label(dsa_k->jump_l, kase)
--
-- where l is level of block in which local label implicitly declared,
-- kase is the sequential number of label in that block (this number
-- used as the second parameter to longjmp to perform global go to).
-- The library routine make_label uses two parameters to make struct
-- label that can be considered as "value of label".
--
-- If primary expression is identifier of formal label called by value
-- then output code is the same as if it were local variable that has
-- "value of label" (struct label).
--
-- If primary expression is identifier of formal label called by name
-- then output code is the same as if it were formal variable called by
-- name that has "value of label" (struct label). Of course, this value
-- will be evaluated by thunk as value of designational expression.
--
-- If primary expression is an expression enclosed by parenthesis then
-- output code inherits these parenthesis.
--
-- In the other cases output code is the same as generated by the
-- corresponding parsing routines of low level. */

static CODE *primary(void)
{     CODE *code;
      if (t_code == T_REAL)
      {  /* real constant */
         code = new_code();
         if (second_pass)
         {  code->lval = 0;
            code->type = F_REAL;
            /* leading zeros should be removed (to avoid treatment as
               something like octal literal), and ten symbol should be
               replaced by 'e'; we need be careful when something like
               '000#+123' or '#-321' is processed */
            {  char *ptr, *ten;
               for (ptr = t_image; *ptr != '\0'; ptr++)
                  if (*ptr != '0') break;
               assert(*ptr != '\0');
               if (*ptr == TEN_CHAR && ptr != t_image) ptr--;
               ten = strchr(ptr, TEN_CHAR);
               if (ten != NULL) *ten = 'e';
               append(code, "%s%s", ten == ptr ? "1" : "", ptr);
            }
         }
         get_token(/* real constant */);
      }
      else if (t_code == T_INT)
      {  /* integer constant */
         code = new_code();
         if (second_pass)
         {  code->lval = 0;
            code->type = F_INT;
            /* leading zeros should be removed to avoid error treatment
               of integer constant as octal one */
            {  char *ptr;
               for (ptr = t_image; *ptr != '\0'; ptr++)
                  if (*ptr != '0') break;
               if (*ptr == '\0') ptr--;
               append(code, "%s", ptr);
            }
         }
         get_token(/* integer constant */);
      }
      else if (t_code == T_FALSE || t_code == T_TRUE)
      {  /* logical constant false or true */
         code = new_code();
         if (second_pass)
         {  code->lval = 0;
            code->type = F_BOOL;
            append(code, "%s",
               t_code == T_FALSE ? a_false : a_true);
         }
         get_token(/* false or true */);
      }
      else if (t_code == T_IDENT)
      {  /* identifier */
         IDENT *id = look_up(t_image, 0, t_ssn);
         /* we need token[2] hint */
         get_token2();
         if (token[2].code == T_DELIM && token[2].delim == S_BEGSUB)
         {  /* subscripted variable or switch designator */
            /* on the first pass switch designator is processed as if it
               were subscripted variable */
            if (first_pass || !(id->flags & F_SWITCH))
               code = subscripted_variable();
            else
               code = switch_designator();
         }
         else if (token[2].code == T_DELIM && token[2].delim == S_LEFT)
         {  /* function designator (with non-empty parameter part) */
proc:       code = function_designator(0);
         }
         else
         {  /* identifier or function designator (with empty parameter
               part) */
            if (second_pass && (id->flags & F_PROC)) goto proc;
            /* on the first pass function designator is processed as if
               if it were simple variable */
            code = new_code();
            if (second_pass)
            {  switch (id->flags)
               {  case F_REAL:
                     /* real local simple variable */
                  case F_REAL | F_OWN:
                     /* real own simple variable */
                  case F_REAL | F_BYVAL:
                     /* real formal parameter called by value */
                  case F_INT:
                     /* integer local simple variable */
                  case F_INT | F_OWN:
                     /* integer own simple variable */
                  case F_INT | F_BYVAL:
                     /* integer formal parameter called by value */
                  case F_BOOL:
                     /* Boolean local simple variable */
                  case F_BOOL | F_OWN:
                     /* Boolean own simple variable */
                  case F_BOOL | F_BYVAL:
                     /* Boolean formal parameter called by value */
                     code->lval = 1;
                     code->type = id->flags & (F_REAL | F_INT | F_BOOL);
                     if (id->flags & F_OWN)
                        append(code, "%s_%d", id->name, id->block->seqn)
                           ;
                     else
                        append(code, "dsa_%d->%s_%d",
                           dsa_level(id), id->name, id->block->seqn);
                     break;
                  case F_REAL | F_BYNAME:
                     /* real formal parameter called by name */
                     /* only value is processed here */
                     code->lval = 0;
                     code->type = F_REAL;
                     append(code, a_get_real "(");
                     catenate(code, call_by_name(id));
                     append(code, ")");
                     break;
                  case F_INT | F_BYNAME:
                     /* integer formal parameter called by name */
                     /* only value is processed here */
                     code->lval = 0;
                     code->type = F_INT;
                     append(code, a_get_int "(");
                     catenate(code, call_by_name(id));
                     append(code, ")");
                     break;
                  case F_BOOL | F_BYNAME:
                     /* Boolean formal parameter called by name */
                     /* only value is processed here */
                     code->lval = 0;
                     code->type = F_BOOL;
                     append(code, a_get_bool "(");
                     catenate(code, call_by_name(id));
                     append(code, ")");
                     break;
                  case F_LABEL:
                     /* local label */
                     code->lval = 0;
                     code->type = F_LABEL;
                     append(code, a_make_label "(dsa_%d->jump_%d, %d)",
                        dsa_level(id), block_level(id->block), id->dim);
                     break;
                  case F_LABEL | F_BYVAL:
                     /* formal label called by value */
                     code->lval = 0;
                     code->type = F_LABEL;
                     append(code, "dsa_%d->%s_%d",
                        dsa_level(id), id->name, id->block->seqn);
                     break;
                  case F_LABEL | F_BYNAME:
                     /* formal label called by name */
                     code->lval = 0;
                     code->type = F_LABEL;
                     append(code, a_get_label "(");
                     catenate(code, call_by_name(id));
                     append(code, ")");
                     break;
                  default:
                     error("invalid use of identifier `%s' as expressio"
                        "n operand", id->name);
                     code->lval = 0;
                     code->type = F_INT;
                     break;
               }
            }
            get_token(/* id */);
         }
      }
      else if (t_delim(S_LEFT))
      {  /* expression enclosed by parentheses */
         get_token(/* ( */);
         code = expression();
         if (t_delim(S_RIGHT))
            get_token(/* ) */);
         else
            error("missing right parenthesis after expression");
         if (second_pass)
         {  code->lval = 0;
            prepend(code, "(");
            append(code, ")");
         }
      }
      else if (t_code == T_STRING)
      {  error("invalid use of string as expression operand");
         get_token(/* "..." */);
         code = new_code();
      }
      else if (t_code == T_DELIM)
      {  error("invalid use of delimiter `%s' as expression operand",
            t_image);
         get_token(/* delim */);
         code = new_code();
      }
      else
         assert(t_code != t_code);
      return code;
}

/*----------------------------------------------------------------------
-- factor, term, arith_expression, relation, bool_primary, bool_factor,
-- bool_term, implication, simple_expr - parse intermediate expression.
--
-- These routines parse intermediate expression using the following
-- productions:
--
-- <factor> ::= <primary expression>
-- <factor> ::= <factor> ** <primary expression>
-- <term> ::= <factor>
-- <term> ::= <term> * <factor>
-- <term> ::= <term> / <factor>
-- <term> ::= <term> % <factor>
-- <arith expression> ::= <term>
-- <arith expression> ::= + <term>
-- <arith expression> ::= - <term>
-- <arith expression> ::= <arith expression> + <term>
-- <arith expression> ::= <arith expression> - <term>
-- <relation> ::= <arith expression> <rho> <arith expression>
-- <rho> ::= < | <= | = | >= | > | !=
-- <bool primary> ::= <relation>
-- <bool primary> ::= ! <relation>
-- <bool factor> ::= <bool primary>
-- <bool factor> ::= <bool factor> & <bool primary>
-- <bool term> ::= <bool factor>
-- <bool term> ::= <bool term> | <bool factor>
-- <implication> ::= <bool term>
-- <implication> ::= <implication> -> <bool term>
-- <simple expression> ::= <implication>
-- <simple expression> ::= <simple expression> == <implication>
--
-- Each parsing routine of this group parses intermediate expression
-- that corresponds to appropriate non-terminal (exculding <rho>). And
-- all these routines work in the same manner.
--
-- Output code generated by each routine inherits output code of source
-- expression operands connected by appropriate operator (for relations
-- and Boolean expressions operators are implemented through macros to
-- avoid some problems due to "lazy calculations" specific to the C). */

static CODE *factor(void)
{     CODE *x, *y;
      x = primary();
      while (t_delim(S_POWER))
      {  if (second_pass)
         {  if (!(x->type == F_INT || x->type == F_REAL))
            {  error("operand preceding `^' is not of arithmetic type");
               x->type = F_INT;
            }
         }
         get_token(/* ^ */);
         y = primary();
         if (second_pass)
         {  if (!(y->type == F_INT || y->type == F_REAL))
            {  error("operand following `^' is not of arithmetic type");
               y->type = F_INT;
            }
            x->lval = 0;
            if (y->type == F_REAL)
               to_real(x), prepend(x, a_expr "(");
            else if (x->type == F_REAL)
               prepend(x, a_expn "(");
            else
               prepend(x, a_expi "(");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

static CODE *term(void)
{     CODE *x, *y;
      x = factor();
      while (t_delim(S_TIMES) || t_delim(S_SLASH) || t_delim(S_INTDIV))
      {  int op = token[1].delim;
         if (second_pass)
         {  if (!(x->type == F_INT || x->type == F_REAL))
            {  error("operand preceding `*', `/', or `%%' is not of ari"
                  "thmetic type");
               x->type = F_INT;
            }
            if (op == S_INTDIV && x->type != F_INT)
            {  error("operand preceding `%%' is not of integer type");
               x->type = F_INT;
            }
         }
         get_token(/* * or / or % */);
         y = factor();
         if (second_pass)
         {  if (!(y->type == F_INT || y->type == F_REAL))
            {  error("operand following `*', `/', or `%%' is not of ari"
                  "thmetic type");
               y->type = F_INT;
            }
            if (op == S_INTDIV && y->type != F_INT)
            {  error("operand following `%%' is not of integer type");
               y->type = F_INT;
            }
            x->lval = 0;
            if (x->type == F_REAL || op == S_SLASH || y->type == F_REAL)
               to_real(x), to_real(y);
            append(x, " %c ", op == S_TIMES ? '*' : '/');
            catenate(x, y);
         }
      }
      return x;
}

static CODE *arith_expression(void)
{     CODE *x, *y;
      if (t_delim(S_PLUS) || t_delim(S_MINUS))
      {  int op = token[1].delim;
         get_token(/* + or - */);
         x = term();
         if (second_pass)
         {  if (!(x->type == F_INT || x->type == F_REAL))
            {  error("operand following unary `+' or `-' is not of arit"
                  "hmetic type");
               x->type = F_INT;
            }
            x->lval = 0;
            prepend(x, "%c", op == S_PLUS ? '+' : '-');
         }
      }
      else
         x = term();
      while (t_delim(S_PLUS) || t_delim(S_MINUS))
      {  int op = token[1].delim;
         if (second_pass)
         {  if (!(x->type == F_INT || x->type == F_REAL))
            {  error("operand preceding `+' or `-' is not of arithmetic"
                  " type");
               x->type = F_INT;
            }
         }
         get_token(/* + or - */);
         y = term();
         if (second_pass)
         {  if (!(y->type == F_INT || y->type == F_REAL))
            {  error("operand following `+' or `-' is not of arithmetic"
                  " type");
               y->type = F_INT;
            }
            x->lval = 0;
            if (x->type == F_REAL || y->type == F_REAL)
               to_real(x), to_real(y);
            append(x, " %c ", op == S_PLUS ? '+' : '-');
            catenate(x, y);
         }
      }
      return x;
}

static CODE *relation(void)
{     CODE *x, *y;
      int flag = 0;
      /* context ... <rho> ... <rho> ... is invalid because <relation>
         can't be operand of relational operator; nevertheless such
         context is processed as if it were valid to correct error */
      x = arith_expression();
      while (t_delim(S_LESS) || t_delim(S_NOTGREATER) ||
             t_delim(S_EQUAL) || t_delim(S_NOTLESS) ||
             t_delim(S_GREATER) || t_delim(S_NOTEQUAL))
      {  int op = token[1].delim;
         if (flag)
            error("invalid use of relational operator");
         flag = 1;
         if (second_pass)
         {  if (!(x->type == F_INT || x->type == F_REAL))
            {  error("operand preceding relational operator is not of a"
                  "rithmetic type");
               x->type = F_INT;
            }
         }
         get_token(/* <rho> */);
         y = arith_expression();
         if (second_pass)
         {  if (!(y->type == F_INT || y->type == F_REAL))
            {  error("operand following relational operator is not of a"
                  "rithmetic type");
               y->type = F_INT;
            }
            if (x->type == F_REAL || y->type == F_REAL)
               to_real(x), to_real(y);
            x->lval = 0;
            x->type = F_BOOL;
            prepend(x, "%s(",
               op == S_LESS       ? a_less       :
               op == S_NOTGREATER ? a_notgreater :
               op == S_EQUAL      ? a_equal      :
               op == S_NOTLESS    ? a_notless    :
               op == S_GREATER    ? a_greater    :
               op == S_NOTEQUAL   ? a_notequal   : "???");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

static CODE *bool_primary(void)
{     CODE *x;
      if (!t_delim(S_NOT))
         x = relation();
      else
      {  get_token(/* ! */);
         x = relation();
         if (second_pass)
         {  if (x->type != F_BOOL)
            {  error("operand following `!' is not of Boolean type");
               x->type = F_BOOL;
            }
            x->lval = 0;
            prepend(x, a_not "(");
            append(x, ")");
         }
      }
      return x;
}

static CODE *bool_factor(void)
{     CODE *x, *y;
      x = bool_primary();
      while (t_delim(S_AND))
      {  if (second_pass)
         {  if (x->type != F_BOOL)
            {  error("operand preceding `&' is not of Boolean type");
               x->type = F_BOOL;
            }
         }
         get_token(/* & */);
         y = bool_primary();
         if (second_pass)
         {  if (y->type != F_BOOL)
            {  error("operand following `&' is not of Boolean type");
               y->type = F_BOOL;
            }
            x->lval = 0;
            prepend(x, a_and "(");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

static CODE *bool_term(void)
{     CODE *x, *y;
      x = bool_factor();
      while (t_delim(S_OR))
      {  if (second_pass)
         {  if (x->type != F_BOOL)
            {  error("operand preceding `|' is not of Boolean type");
               x->type = F_BOOL;
            }
         }
         get_token(/* & */);
         y = bool_factor();
         if (second_pass)
         {  if (y->type != F_BOOL)
            {  error("operand following `|' is not of Boolean type");
               y->type = F_BOOL;
            }
            x->lval = 0;
            prepend(x, a_or "(");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

static CODE *implication(void)
{     CODE *x, *y;
      x = bool_term();
      while (t_delim(S_IMPL))
      {  if (second_pass)
         {  if (x->type != F_BOOL)
            {  error("operand preceding `->' is not of Boolean type");
               x->type = F_BOOL;
            }
         }
         get_token(/* & */);
         y = bool_term();
         if (second_pass)
         {  if (y->type != F_BOOL)
            {  error("operand following `->' is not of Boolean type");
               y->type = F_BOOL;
            }
            x->lval = 0;
            prepend(x, a_impl "(");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

static CODE *simple_expr(void)
{     CODE *x, *y;
      x = implication();
      while (t_delim(S_EQUIV))
      {  if (second_pass)
         {  if (x->type != F_BOOL)
            {  error("operand preceding `==' is not of Boolean type");
               x->type = F_BOOL;
            }
         }
         get_token(/* & */);
         y = implication();
         if (second_pass)
         {  if (y->type != F_BOOL)
            {  error("operand following `==' is not of Boolean type");
               y->type = F_BOOL;
            }
            x->lval = 0;
            prepend(x, a_equiv "(");
            append(x, ", ");
            catenate(x, y);
            append(x, ")");
         }
      }
      return x;
}

/*----------------------------------------------------------------------
-- expression - parse general expression.
--
-- This routine parse expression of general kind using the syntax:
--
-- <expression> ::= <simple expression>
-- <expression> ::= if <expression> then <simple expression> else
--                  <expression>
--
-- The first case is obvious. In the second case output code has the
-- form:
--
-- ((condition) ? (simple expression) : (expression))
--
-- where condition is expression following 'if' symbol. */

static CODE *expression(void)
{     CODE *x;
      if (!t_delim(S_IF))
         x = simple_expr();
      else
      {  CODE *sae, *ae;
         get_token(/* if */);
         x = expression();
         if (!t_delim(S_THEN))
            error("missing `then' delimiter");
         if (second_pass && x->type != F_BOOL)
            error("expression following `if' is not of Boolean type");
         if (t_delim(S_THEN)) get_token(/* then */);
         sae = simple_expr(); /* expression before else */
         if (t_delim(S_ELSE))
            get_token(/* else */);
         else
            error("missing `else' delimiter");
         ae = expression(); /* expression after else */
         if (second_pass)
         {  if (sae->type == F_INT && ae->type == F_REAL) to_real(sae);
            if (sae->type == F_REAL && ae->type == F_INT) to_real(ae);
            if (sae->type != ae->type)
               error("expressions before and after 'else' incompatible")
                  ;
            x->lval = 0;
            x->type = sae->type;
            prepend(x, "((");
            append(x, ") ? (");
            catenate(x, sae);
            append(x, ") : (");
            catenate(x, ae);
            append(x, "))");
         }
      }
      return x;
}

/*----------------------------------------------------------------------
-- assignment_statement - parse assignment statement.
--
-- This routine parses assignment statement using the syntax:
--
-- <assignment statement> ::= <destination> := <expression>
-- <assignment statement> ::= <destination> := <assignment statement>
-- <destination> ::= <simple variable identifier>
-- <destination> ::= <formal parameter identifier>
-- <destination> ::= <procedure identifier>
-- <destination> ::= <subscripted variable>
--
-- If flag is clear then the routine is called on statement level.
-- Otherwise if flag is set the routine is called after ':=' delimiter.
--
-- If destination is simple local or own variable or simple formal
-- parameter called by value then output code has the form:
--
-- dsa_k->id_n = expression (or id_n = expression for own variables)
--
-- where k is level of DSA in which identifier is placed, and n is the
-- number of block used as suffix.
--
-- If destination is simple formal parameter called by name then the
-- output code has the form:
--
-- set_xxx((global_dsa = arg.arg2, arg.arg1()), expression)
--
-- where set_xxx is library routine set_real, set_int, or set_bool,
-- that uses struct desc (returned by thunk) to assign expression value
-- to the appropriate variable. Each routine set_xxx returns the value
-- of expression so it can be used to further assignment.
--
-- If destination is procedure identifier (in this case assignment
-- statement must be located inside the procedure body) then the output
-- code has the form:
--
-- dsa_k->retval.u.xxx_val = expression
--
-- where retval is object of struct desc type used as value returned
-- by procedure.
--
-- If destination is subscripted variable then the output code has the
-- form:
--
-- *loc_xxx(dv, n, i1, i2, ..., in) = expression
--
-- (see also the routine subscripted_variable). */

static CODE *assignment_statement(int flag)
{     CODE *x;
      if (t_code == T_IDENT) get_token2();
      if (t_code == T_IDENT &&
          token[2].code == T_DELIM && token[2].delim == S_ASSIGN)
      {  /* the current context has the form id := ... */
         IDENT *id = look_up(t_image, 0, t_ssn);
         if (first_pass)
         {  /* skip all semantic checks */
            get_token(/* id */), get_token(/* := */);
            assignment_statement(1);
            goto skip1;
         }
         /* identifier must denote only simple variable, simple formal
            parameter, or type procedure */
         if (id->flags & F_LABEL)
            error("invalid use of label `%s' in left part of assignment"
               " statement", id->name);
         else if (id->flags & F_ARRAY)
            error("invalid use of array identifier `%s' in left part of"
               " assignment statement", id->name);
         else if (id->flags & F_SWITCH)
            error("invalid use of switch identifier `%s' in left part o"
               "f assignment statement", id->name);
         else if (id->flags & F_STRING)
            error("invalid use of formal string `%s' in left part of as"
               "signment statement", id->name);
         else if (id->flags & F_PROC)
         {  /* if identifier denotes procedure then assignment statement
               must be located only inside body of that procedure */
            BLOCK *b;
            for (b = current; b != NULL; b = b->surr)
               if (b->proc == id) break;
            if (b == NULL)
               error("invalid assignment to procedure identifier `%s' o"
                  "utside procedure declaration body", id->name);
            /* and procedure must have type */
            if (!(id->flags & (F_REAL | F_INT | F_BOOL)))
               error("invalid use of typeless procedure identifier `%s'"
                  " in left part of assignment statement", id->name);
         }
         get_token(/* id */), get_token(/* := */);
         /* parse the rest part of assignment statement that can be
            again assignment statement or final expression; this is
            resolved by means of lval flag */
         x = assignment_statement(1);
         /* clear lvalue flag if x is final expression */
         if (!t_delim(S_ASSIGN)) x->lval = 0;
         /* type conversion is allowed only for final expression */
         if (x->lval == 0)
         {  /* after delimiter := final expression detected */
            if ((id->flags & F_REAL) && x->type == F_INT)  to_real(x);
            if ((id->flags & F_INT)  && x->type == F_REAL) to_int(x);
            if ((id->flags & (F_REAL | F_INT | F_BOOL)) != x->type)
               error("type of identifier `%s' in left part of assignmen"
                  "t statement incompatible with type of assigned expre"
                  "ssion", id->name);
         }
         else
         {  /* after delimiter := assignement statement detected; type
               conversion is not allowed */
            if ((id->flags & (F_REAL | F_INT | F_BOOL)) != x->type)
               error("different types in left part list of assignment s"
                  "tatement");
         }
         /* generate code */
         switch (id->flags)
         {  case F_REAL:
               /* real local simple variable */
            case F_REAL | F_OWN:
               /* real own simple variable */
            case F_REAL | F_BYVAL:
               /* real simple formal parameter called by value */
            case F_INT:
               /* integer local simple variable */
            case F_INT | F_OWN:
               /* integer own simple variable */
            case F_INT | F_BYVAL:
               /* integer simple formal parameter called by value */
            case F_BOOL:
               /* Boolean local simple variable */
            case F_BOOL | F_OWN:
               /* Boolean own simple variable */
            case F_BOOL | F_BYVAL:
               /* Boolean simple formal parameter called by value */
               x->lval = 1; /* mark assignment statement */
               x->type = id->flags & (F_REAL | F_INT | F_BOOL);
               if (id->flags & F_OWN)
                  prepend(x, "%s_%d = ", id->name, id->block->seqn);
               else
                  prepend(x, "dsa_%d->%s_%d = ",
                     dsa_level(id), id->name, id->block->seqn);
               break;
            case F_REAL | F_BYNAME:
               /* real simple formal parameter called by name */
               {  CODE *code = call_by_name(id);
                  prepend(code, a_set_real "(");
                  append(code, ", ");
                  catenate(code, x);
                  append(code, ")");
                  x = code;
                  x->lval = 1; /* mark assignment statement */
                  x->type = F_REAL;
               }
               break;
            case F_INT | F_BYNAME:
               /* integer simple formal parameter called by name */
               {  CODE *code = call_by_name(id);
                  prepend(code, a_set_int "(");
                  append(code, ", ");
                  catenate(code, x);
                  append(code, ")");
                  x = code;
                  x->lval = 1; /* mark assignment statement */
                  x->type = F_INT;
               }
               break;
            case F_BOOL | F_BYNAME:
               /* Boolean simple formal parameter called by name */
               {  CODE *code = call_by_name(id);
                  prepend(code, a_set_bool "(");
                  append(code, ", ");
                  catenate(code, x);
                  append(code, ")");
                  x = code;
                  x->lval = 1; /* mark assignment statement */
                  x->type = F_BOOL;
               }
               break;
            case F_REAL | F_PROC:
               /* real local procedure */
            case F_INT | F_PROC:
               /* integer local procedure */
            case F_BOOL | F_PROC:
               /* Boolean local procedure */
               x->lval = 1; /* mark assignment statement */
               x->type = id->flags & (F_REAL | F_INT | F_BOOL);
               prepend(x, "dsa_%d->retval.u.%s = ", dsa_level(id)+1,
                  x->type == F_REAL ? "real_val" :
                  x->type == F_INT  ? "int_val"  :
                  x->type == F_BOOL ? "bool_val" : "???");
               break;
            default:
               /* error diagnostics has yet generated */
               break;
         }
skip1:   ;
      }
      else
      {  /* the current context or begins left part (which must be only
            subscripted variable), or is final expression */
         CODE *y;
         x = expression();
         if (t_delim(S_ASSIGN))
         {  /* the current context has the form ... := ergo expression
               must be only subscripted variable */
            if (second_pass && !x->lval)
               error("invalid use of delimiter `:=' after expression in"
                  " assignment statement");
            get_token(/* := */);
            y = assignment_statement(1);
            /* igitur, x := y */
            if (first_pass) goto skip2;
            /* type conversion is allowed only for final expression */
            if (y->lval == 0)
            {  /* after delimiter := final expression detected */
               if (x->type == F_REAL && y->type == F_INT) to_real(y);
               if (x->type == F_INT && y->type == F_REAL) to_int(y);
               if (x->type != y->type)
                  error("type of destination in left part of assignment"
                     " statement incompatible with type of assigned exp"
                     "ression");
            }
            else
            {  /* after delimiter := assignement statement detected;
                  type conversion is not allowed */
               if (x->type != y->type)
                  error("different types in left part list of assignmen"
                     "t statement");
            }
            /* generate code */
            x->lval = 1; /* mark assignment statement */
            append(x, " = ");
            catenate(x, y);
skip2:      ;
         }
         else
         {  /* final expression reached */
            if (first_pass) goto skip3;
            /* it is allowed only after := delimiter */
            if (!flag)
            {  error("invalid use of expression");
               goto skip3;
            }
            if (!(x->type == F_REAL || x->type == F_INT ||
                  x->type == F_BOOL))
            {  error("invalid type of assigned expression in assignment"
                  " statement");
               x->type = F_REAL;
            }
            /* x is final expression, so clear its lvalue flag */
            x->lval = 0;
skip3:      ;
         }
      }
      if (!flag)
      {  prepend(x, "      ");
         append(x, ";\n");
      }
      return x;
}

/*----------------------------------------------------------------------
-- go_to_statement - parse go to statement.
--
-- This routine parses go to statement using the syntax:
--
-- <go to statement> ::= go to <designational expression>
--
-- If and only if designational expression is a label which is located
-- in the same block as go to statement it is possible to perform go to
-- directly. In this case output code has the form:
--
-- goto id_n
--
-- where id_n is label identifier suffixed by sequential number of
-- block in which the label is localized.
--
-- In all other cases output code has the form:
--
-- go_to(expression)
--
-- where go_to is a library routine that uses expression as the value
-- of struct label to perform global go to. */

static CODE *go_to_statement(void)
{     CODE *code;
      /* the current token must be 'go to' */
      assert(t_delim(S_GOTO));
      get_token(/* go to */);
      /* if delimiter 'go to' is followed by label identifier (but not
         general designational expression) and if this label is located
         in the current block then direct go to is possible*/
      if (second_pass && t_code == T_IDENT)
      {  get_token2();
         if (token[2].code == T_DELIM &&
            (token[2].delim == S_SEMICOLON ||
             token[2].delim == S_ELSE || token[2].delim == S_END))
         {  IDENT *id = look_up(t_image, 0, t_ssn);
            if (!(id->flags & F_LABEL))
               error("invalid use identifier `%s' as a label in go to s"
                  "tatement", id->name);
            if (id->flags == F_LABEL && id->block == current)
            {  /* go to local label in the same block */
               code = new_code();
               append(code, "      goto %s_%d;\n",
                  id->name, id->block->seqn);
               get_token(/* id */);
               goto done;
            }
         }
      }
      /* otherwise general designational expression is used */
      code = expression();
      if (second_pass)
      {  if (code->type != F_LABEL)
            error("expression following `go to' is not of label type");
         prepend(code, "      " a_go_to "(");
         append(code, ");\n");
      }
done: return code;
}

/*----------------------------------------------------------------------
-- dummy_statement - parse dummy statement.
--
-- This routine parses dummy statement using the syntax:
--
-- <dummy statement> ::= <empty>
--
-- Output code has obvious form. */

static CODE *dummy_statement(void)
{     CODE *code = new_code();
      append(code, "      /* <dummy statement> */;\n");
      return code;
}

/*----------------------------------------------------------------------
-- emit_ssn - generate source line number.
--
-- This routine generates output code to remember source line number
-- ssn in DSA of current procedure. */

static CODE *emit_ssn(int ssn)
{     CODE *code = new_code();
      append(code, "      dsa_%d->line = %d;\n", current_level(), ssn);
      return code;
}

/*----------------------------------------------------------------------
-- label_list - parse label list.
--
-- This routine parses list of labels optionally preceding a statement
-- using the syntax:
--
-- <label list> ::= <empty>
-- <label list> ::= <label list> <label> :
--
-- Each label detected by this routine will be implicitly declared in
-- the current block.
--
-- Note that this routine called before parsing of each statement. */

static CODE *label_list(void)
{     CODE *code = new_code();
      if (t_delim(S_ELSE) || t_delim(S_END) ||
          t_delim(S_SEMICOLON))
         warning("unlabelled dummy statement");
      for (;;)
      {  if (t_code == T_IDENT || t_code == T_INT) get_token2();
         if (t_code == T_IDENT &&
            (token[2].code == T_DELIM && token[2].delim == S_COLON))
         {  /* declare label identifier implicitly */
            IDENT *label;
            label = look_up(t_image, 1, t_ssn);
            label->flags = F_LABEL; /* local label */
            append(code, "%s_%d:\n", label->name, label->block->seqn);
            get_token(/* id */), get_token(/* : */);
         }
         else if (t_code == T_INT &&
            (token[2].code == T_DELIM && token[2].delim == S_COLON))
         {  /* valid in the Revised Report; invalid in the Modified
               Report */
            error("invalid use unsigned integer `%s' as a label",
               t_image);
            get_token(/* integer */), get_token(/* : */);
         }
         else
            break;
      }
      /* since this routine is called before parsing of each statement,
         it is the time to insert source line number to output code */
      catenate(code, emit_ssn(t_ssn));
      return code;
}

/*----------------------------------------------------------------------
-- conditional_statement - parse conditional statement.
--
-- This routine parses conditional statement uses the syntax:
--
-- <conditional statement> ::= if <expression> then <label list>
--                             <statement>
-- <conditional statement> ::= if <expression> then <label list>
--                             <statement> else <label list> <statement>
--
-- Note that statement preceding else cannot be conditional statement or
-- for statement due to restriction of the reference language.
--
-- In the first case output code has the form:
--
-- if (!(expression)) goto _omega_n;
-- statement;
-- _omega_n:
--
-- In the second case output code has the form:
--
-- if (!(expression)) goto _gamma_n;
-- statement following then;
-- goto _omega_n;
-- _gamma_n:
-- statement following else;
-- _omega_n:
--
-- where n is label count used as unique suffix. */

static CODE *statement(void);

static int label_count = 0;
/* Label count. This value is used as suffix to form unique names of
   auxiliary labels. */

static CODE *conditional_statement(void)
{     CODE *code, *then_part, *else_part;
      int no_else;
      /* the current token must be 'if' */
      assert(t_delim(S_IF));
      get_token(/* if */);
      /* parse expression following 'if' */
      code = expression();
      if (!t_delim(S_THEN))
         error("missing `then' delimiter");
      if (second_pass && code->type != F_BOOL)
         error("expression following `if' is not of Boolean type");
      /* parse statement following 'then' */
      if (t_delim(S_THEN)) get_token(/* then */);
      then_part = label_list();
      no_else = (t_delim(S_IF) || t_delim(S_FOR));
      catenate(then_part, statement());
      /* parse optional else part and generate code */
      if (!t_delim(S_ELSE))
      {  /* statement has the form 'if ... then ...' */
         if (second_pass)
         {  prepend(code, "      if (!(");
            label_count++;
            append(code, ")) goto _omega_%d;\n", label_count);
            catenate(code, then_part);
            append(code, "_omega_%d:\n", label_count);
         }
      }
      else
      {  /* statement has the form 'if ... then ... else ...' */
         if (no_else)
            error("invalid use of delimiter `else' after if or for stat"
               "ement");
         get_token(/* else */);
         else_part = label_list();
         catenate(else_part, statement());
         if (second_pass)
         {  prepend(code, "      if (!(");
            if (second_pass) label_count++;
            append(code, ")) goto _gamma_%d;\n", label_count);
            catenate(code, then_part);
            append(code, "      goto _omega_%d;\n_gamma_%d:\n",
               label_count, label_count);
            catenate(code, else_part);
            append(code, "_omega_%d:\n", label_count);
         }
      }
      return code;
}

/*----------------------------------------------------------------------
-- get_variable - generate code to get value of controlled variable.
--
-- This routine generates code to get value of controlled variable id.
-- Output code has the same form as if the controlled variable were used
-- in expression. */

static CODE *get_variable(IDENT *id)
{     CODE *expr = new_code();
      if (second_pass)
      {  switch (id->flags)
         {  case F_REAL:
               /* real simple local variable */
            case F_REAL | F_OWN:
               /* real simple own variable */
            case F_REAL | F_BYVAL:
               /* real simple formal parameter called by value */
            case F_INT:
               /* integer simple local variable */
            case F_INT | F_OWN:
               /* integer simple own variable */
            case F_INT | F_BYVAL:
               /* integer simple formal parameter called by value */
               expr->lval = 0;
               expr->type = id->flags & (F_REAL | F_INT | F_BOOL);
               if (id->flags & F_OWN)
                  append(expr, "%s_%d", id->name, id->block->seqn);
               else
                  append(expr, "dsa_%d->%s_%d",
                     dsa_level(id), id->name, id->block->seqn);
               break;
            case F_REAL | F_BYNAME:
               /* real simple formal parameter called by name */
               expr->lval = 0;
               expr->type = F_REAL;
               append(expr, a_get_real "(");
               catenate(expr, call_by_name(id));
               append(expr, ")");
               break;
            case F_INT | F_BYNAME:
               /* integer simple formal parameter called by name */
               expr->lval = 0;
               expr->type = F_INT;
               append(expr, a_get_int "(");
               catenate(expr, call_by_name(id));
               append(expr, ")");
               break;
            default:
               /* invalid controlled variable */
               append(expr, "???");
               break;
            }
         }
         return expr;
}

/*----------------------------------------------------------------------
-- set_variable - generate code to set value of controlled variable.
--
-- This routine generates code to assign value of expression expr to
-- controlled variable id. Output code has the same form as if the
-- controlled variable were used in left part of assignment statement.*/

static CODE *set_variable(IDENT *id, CODE *expr)
{     if (second_pass)
      {  if ((id->flags & F_REAL) && expr->type == F_INT) to_real(expr);
         if ((id->flags & F_INT)  && expr->type == F_REAL) to_int(expr);
         switch (id->flags)
         {  case F_REAL:
               /* real simple local variable */
            case F_REAL | F_OWN:
               /* real simple own variable */
            case F_REAL | F_BYVAL:
               /* real simple formal parameter called by value */
            case F_INT:
               /* integer simple local variable */
            case F_INT | F_OWN:
               /* integer own local variable */
            case F_INT | F_BYVAL:
               /* integer simple formal parameter called by value */
               expr->type = id->flags & (F_REAL | F_INT | F_BOOL);
               if (id->flags & F_OWN)
                  prepend(expr, "%s_%d = ", id->name, id->block->seqn);
               else
                  prepend(expr, "dsa_%d->%s_%d = ",
                     dsa_level(id), id->name, id->block->seqn);
               break;
            case F_REAL | F_BYNAME:
               /* real simple formal parameter called by name */
               {  CODE *code = call_by_name(id);
                  prepend(code, a_set_real "(");
                  append(code, ", ");
                  catenate(code, expr);
                  append(code, ")");
                  expr = code;
                  expr->lval = 1; /* mark assignment statement */
                  expr->type = F_REAL;
               }
               break;
            case F_INT | F_BYNAME:
               /* integer simple formal parameter called by name */
               {  CODE *code = call_by_name(id);
                  prepend(code, a_set_int "(");
                  append(code, ", ");
                  catenate(code, expr);
                  append(code, ")");
                  expr = code;
                  expr->lval = 1; /* mark assignment statement */
                  expr->type = F_INT;
               }
               break;
            default:
               /* invalid controlled variable */
               append(expr, "???");
               break;
         }
         prepend(expr, "      ");
         append(expr, ";\n");
      }
      return expr;
}

/*----------------------------------------------------------------------
-- for_statement - parse for statement.
--
-- This routine parses for statement using the syntax:
--
-- <for statement> ::= for <identifier> := <for list> do <label list>
--                     <statement>
-- <for list> ::= <for element>
-- <for list> ::= <for list> , <for element>
-- <for element> ::= <expression>
-- <for element> ::= <expression> step <expression> until <expression>
-- <for element> ::= <expression> while <expression>
--
-- Labelled statement following 'do' is always translated to separate
-- static routine (although it is actually needed when for list has two
-- or more for elements).
--
-- In case of arithmetic expression element a for statement
--
-- for V := X do S
--
-- translates to output code that has the form:
--
-- V = X;
-- _sigma_k();
--
-- where _sigma_k() is translated version of the statement S.
--
-- In case of step-until element a for statement
--
-- for V := A step B until C do S
--
-- translates to output code that has the form:
--
-- V = A;
-- _gamma_n:
-- teta = B;
-- if ((V - C) * sign(teta) > 0) goto _omega_n;
-- _sigma_k();
-- V = V + teta;
-- goto _gamma_n;
-- _omega_n: (* element exhausted *)
--
-- where teta is auxiliary local variable of the same type as B.
--
-- In case of while element a for statement
--
-- for V := E while F do S
--
-- translates to output code that has the form:
--
-- _gamma_n:
-- V = E;
-- if (!E) goto _omega_n;
-- _sigma_k();
-- goto _gamma_n;
-- _omega_n:
--
-- Note that although explicit form of subscripted controlled variable
-- is not allowed, in this implementation controlled variable can be a
-- formal parameter called by name where the corresponding actual
-- parameter is subscripted variable. (Of course, since the reference
-- language uses static treatment of the call by name as substitution,
-- this feature should be considered as non-standard extension.) */

static CODE *enter_block(IDENT *proc, int ssn);
static CODE *leave_block(void);

static int for_count = 0;

static CODE *for_statement(void)
{     IDENT *id;
      CODE *code = new_code();
      int count = first_pass ? 0 : ++for_count;
      /* the current token must be 'for' */
      assert(t_delim(S_FOR));
      get_token(/* for */);
      /* check controlled variable identifier */
      if (t_code == T_IDENT)
      {  id = look_up(t_image, 0, t_ssn);
         get_token(/* id */);
      }
      else
      {  char str[50];
         sprintf(str, "i_%d", t_ssn);
         error("missing controlled variable identifier after `for'; dum"
            "my identifier `%s' used", str);
         id = look_up(t_image, 1, t_ssn);
         id->ssn_decl = id->ssn_used = t_ssn;
         id->flags = F_REAL;
      }
      if (t_delim(S_BEGSUB))
         error("subscripted controlled variable not allowed");
      if (t_delim(S_ASSIGN))
         get_token(/* := */);
      else
         error("missing ':=' after controlled variable identifier");
      /* check controlled variable identifier */
      if (second_pass)
      {  if (id->flags & (F_LABEL | F_ARRAY | F_SWITCH | F_PROC |
            F_STRING))
            error("invalid use of identifier `%s' as controlled variabl"
               "e", id->name);
         else if (!(id->flags & (F_REAL | F_INT)))
            error("invalid type of controlled variable `%s'", id->name);
      }
loop: /* translate the current for list element */
      catenate(code, emit_ssn(t_ssn));
      /* V := expression */
      {  CODE *expr = expression();
         if (second_pass)
         {  if (!(expr->type == F_REAL || expr->type == F_INT))
            {  error("invalid type of expression assigned to controlled"
                  " variable");
               expr->type = F_REAL;
            }
            /* if expression is followed by 'while' then we should
               generate an auxiliary label to repeat assignment value of
               expression to the controlled variable */
            if (t_delim(S_WHILE))
            {  label_count++;
               append(code, "_gamma_%d:\n", label_count);
               catenate(code, emit_ssn(t_ssn));
            }
            catenate(code, set_variable(id, expr));
         }
      }
      /* parse for list element */
      if (t_delim(S_COMMA) || t_delim(S_DO))
      {  /* arithmetic expression element */
         /* generate code to perform statement following 'do' */
         append(code,
            "      " a_global_dsa " = (void *)dsa_%d, _sigma_%d();\n",
            current_level(), count);
      }
      else if (t_delim(S_STEP))
      {  /* step-until element */
         /* A step B until C */
         IDENT *teta;
         CODE *expr;
         /* declare an auxiliary variable teta that has the same type
            as expression B; this variable is declared in the current
            block and since possible internal for statement will be
            in dummy block that encloses statement following 'do', then
            teta-variables for different for statement are declared
            always in different blocks. */
         /* on the first pass, when we need to declare teta, the type
            of B is unknown yet; that's why two variables teta_i of
            integer type and teta_r of real type are declared */
         if (first_pass)
         {  teta = look_up("teta_r", 0, t_ssn);
            if (teta->ssn_decl == 0) teta->ssn_decl = t_ssn;
            teta->flags = F_REAL;
            teta = look_up("teta_i", 0, t_ssn);
            if (teta->ssn_decl == 0) teta->ssn_decl = t_ssn;
            teta->flags = F_INT;
         }
         /* parse expression B following 'step' */
         get_token(/* step */);
         expr = expression();
         /* select actual teta variable depending on type of B */
         if (second_pass)
         {  if (expr->type == F_REAL)
               teta = look_up("teta_r", 0, 0);
            else if (expr->type == F_INT)
               teta = look_up("teta_i", 0, 0);
            else
            {  error("expression following `step' is not of arithmetic "
                  "type");
               teta = look_up("teta_r", 0, 0);
            }
            assert(teta != NULL && teta->block == current);
         }
         /* teta := B */
         if (second_pass)
         {  append(code, "      dsa_%d->%s_%d = ",
               current_level(), teta->name, teta->block->seqn);
            catenate(code, expr);
            append(code, ";\n");
         }
         /* generate code to check condition for break a loop */
         /* gamma: if (V - C) * sign(teta) > 0 then goto omega */
         if (second_pass)
         {  label_count++;
            append(code, "_gamma_%d:\n", label_count);
            catenate(code, emit_ssn(t_ssn));
         }
         /* parse expression C following 'until' */
         if (t_delim(S_UNTIL))
            get_token(/* until */);
         else
            error("missing `until' delimiter");
         expr = expression();
         /* convert C to type of V */
         if (second_pass)
         {  if ((id->flags & F_REAL) && expr->type == F_INT)
               to_real(expr);
            if ((id->flags & F_INT) && expr->type == F_REAL)
               to_int(expr);
            if (!(expr->type == F_REAL || expr->type == F_INT))
            {  error("expression following `until' is not of arithmetic"
                  " type");
               expr->type = F_REAL;
            }
         }
         /* now we should be very careful about type conversions */
         if (second_pass)
         {  append(code, "      if ((");
            catenate(code, get_variable(id));
            append(code, " - (");
            catenate(code, expr);
            if (id->flags & F_REAL)
               append(code, ")) * (double)(");
            else
               append(code, ")) * (");
            if (teta->flags & F_REAL)
               append(code, "dsa_%d->%s_%d < 0.0 ? -1 : dsa_%d->%s_%d >"
                  " 0.0 ? +1 : 0",
                  current_level(), teta->name, teta->block->seqn,
                  current_level(), teta->name, teta->block->seqn);
            else
               append(code, "dsa_%d->%s_%d < 0 ? -1 : dsa_%d->%s_%d > 0"
                  " ? +1 : 0",
                  current_level(), teta->name, teta->block->seqn,
                  current_level(), teta->name, teta->block->seqn);
            if (id->flags & F_REAL)
               append(code, ") > 0.0) ");
            else
               append(code, ") > 0) ");
            append(code, "goto _omega_%d;\n", label_count);
         }
         /* generate code to perform statement following 'do' */
         append(code,
            "      " a_global_dsa " = (void *)dsa_%d, _sigma_%d();\n",
            current_level(), count);
         /* V := V + teta */
         if (second_pass)
         {  /* and again we should be very careful */
            expr = new_code();
            expr->lval = 0;
            expr->type = (teta->flags & (F_REAL | F_INT));
            append(expr, "dsa_%d->%s_%d",
               current_level(), teta->name, teta->block->seqn);
            if ((id->flags & F_REAL) && (teta->flags & F_INT))
               to_real(expr);
            if ((id->flags & F_INT) && (teta->flags & F_REAL))
               to_int(expr);
            append(expr, " + ");
            catenate(expr, get_variable(id));
            catenate(code, set_variable(id, expr));
         }
         /* go to gamma */
         append(code, "      goto _gamma_%d;\n", label_count);
         /* element exhausted */
         append(code, "_omega_%d: /* element exhausted */\n",
            label_count);
      }
      else if (t_delim(S_WHILE))
      {  /* while element */
         /* E while F */
         CODE *expr;
         get_token(/* while */);
         expr = expression();
         if (second_pass)
         {  if (expr->type != F_BOOL)
            {  error("expression following `while' is not of Boolean ty"
                  "pe");
               expr->type = F_BOOL;
            }
            /* label gamma has been generated earlier */
            append(code, "      if (!(");
            catenate(code, expr);
            append(code, ")) goto _omega_%d;\n", label_count);
         }
         /* generate code to perform statement following 'do' */
         append(code,
            "      " a_global_dsa " = (void *)dsa_%d, _sigma_%d();\n",
            current_level(), count);
         /* generate code to make a loop */
         append(code, "      goto _gamma_%d;\n", label_count);
         append(code, "_omega_%d:\n", label_count);
      }
      /* if for list element is followed by comma then parsing of for
         list is continued */
      if (t_delim(S_COMMA))
      {  get_token(/* , */);
         goto loop;
      }
      /* generate and emit code for statement following 'do' (this
         statement is translated to separate static routine) */
      if (!t_delim(S_DO))
         error("missing `do' delimiter after for list");
      {  CODE *stmt;
         int ssn = t_ssn;
         /* enter to dummy block that encloses do statement */
         stmt = enter_block(NULL, t_ssn);
         /* we should save new stack_top value */
         append(stmt, "      dsa_%d->new_top_%d = " a_stack_top ";\n",
            current_level(), block_level(current));
         if (t_delim(S_DO)) get_token(/* do */);
         /* parse statement */
         catenate(stmt, label_list());
         catenate(stmt, statement());
         /* leave dummy block that encloses do statement */
         catenate(stmt, leave_block());
         /* generate and emit code for routine sigma */
         append(emit, "static void _sigma_%d(void)\n", count);
         append(emit,
            "{     /* statement following 'do' at line %d */\n", ssn);
         emit_dsa_pointers();
         catenate(emit, stmt);
         append(emit, "      return;\n");
         append(emit, "}\n");
         append(emit, "\n");
      }
      return code;
}

/*----------------------------------------------------------------------
-- procedure_statement - parse procedure statement.
--
-- This routine parses procedure statement using the syntax:
--
-- <procedure statement> ::= <function designator>
--
-- Since procedure statement is syntactically equivalent to function
-- designator then all work is performed by the routine for function
-- designator. However in special case of pseudo procedure inline or
-- print procedure statement is parsed by this routine. */

static CODE *procedure_statement(void)
{     CODE *code;
      IDENT *proc;
      /* current token must be identifier */
      assert(t_code == T_IDENT);
      proc = look_up(t_image, 0, t_ssn);
      if (second_pass && strcmp(proc->name, "inline") == 0 &&
         (proc->flags & F_BLTIN))
      {  /* pseudo procedure inline */
         get_token(/* id */);
         if (!t_delim(S_LEFT))
err:     {  error("invalid use of pseudo procedure `inline'; translatio"
               "n terminated");
            exit(EXIT_FAILURE);
         }
         get_token(/* ( */);
         if (t_code != T_STRING) goto err;
         code = new_code();
         append(code, "      /* inline code */\n      ");
         {  /* remove enclosing quotes and backslashes */
            char *p = t_image, *q = t_image+1;
            for (;;)
            {  if (*q == '"') break;
               if (*q == '\\') q++;
               assert(*q);
               *p++ = *q++;
            }
            *p = '\0';
         }
         append(code, "%s\n", t_image);
         get_token(/* string */);
         if (!t_delim(S_RIGHT) || ext_comma()) goto err;
         get_token(/* ) */);
      }
      else if (second_pass && strcmp(proc->name, "print") == 0 &&
         (proc->flags & F_BLTIN))
      {  /* pseudo procedure print */
         int count = 0;
         char *place;
         get_token(/* id */);
         if (!t_delim(S_LEFT))
         {  error("invalid use of pseudo procedure `print'");
            code = new_code();
            goto done;
         }
         get_token(/* ( */);
         /* generate the head of code */
         code = new_code();
         append(code, "      " a_print "(???");
         /* remember the place to substitute actual parameter count */
         place = strchr(code->tail->str, '?');
         assert(place != NULL);
         /* parse actual parameter list */
         for (;;)
         {  IDENT *id;
            CODE *expr;
            if (t_code == T_IDENT) get_token2();
            if (t_code == T_IDENT && token[2].code == T_DELIM &&
               (token[2].delim == S_COMMA || token[2].delim == S_RIGHT))
            {  /* the current actual parameter is identifier */
               id = look_up(t_image, 0, t_ssn);
            }
            else
            {  /* the current actual parameter is string or expression;
                  name will not be used */
               id = NULL;
            }
            /* generate code to pass actual parameter */
            if (id != NULL && (id->flags & F_ARRAY))
            {  /* array identifier */
               expr = actual_parameter(NULL);
               append(code, ", 0x%04X, ", F_ARRAY);
            }
            else if (id != NULL && (id->flags & F_STRING))
            {  /* formal string identifier */
               expr = actual_parameter(NULL);
               append(code, ", 0x%04X, ", F_STRING);
            }
            else if (t_code == T_STRING)
            {  /* actual string */
               expr = new_code();
               append(expr, a_make_arg "(");
               append(expr, "%s", t_image);
               append(expr, ", NULL)");
               append(code, ", 0x%04X, ", F_STRING);
               get_token(/* string */);
            }
            else
            {  /* in the other cases actual parameter must be in form
                  of expression */
               expr = expression();
               append(code, ", 0x%04X, ", expr->type);
            }
            if (id != NULL)
               append(code, "\"%s\", ", id->name);
            else
               append(code, "NULL, ");
            catenate(code, expr);
            count++;
            /* if the current actual parameter is followed by extended
               parameter delimiter, then parsing of actual parameter
               list is continued */
            if (!ext_comma()) break;
         }
         /* the current token must be right parenthesis */
         assert(t_delim(S_RIGHT));
         get_token(/* ) */);
         append(code, ");\n");
         assert(count <= 255);
         sprintf(place, "%3d", count);
      }
      else
      {  /* conventional procedure */
         code = function_designator(1);
         prepend(code, "      ");
         append(code, ";\n");
      }
done: return code;
}

/*----------------------------------------------------------------------
-- statement - parse unlabeled statement.
--
-- This procedure parses unlabeled statement using the syntax:
--
-- <statement> ::= <block>
-- <statement> ::= <compound statement>
-- <statement> ::= <assignment statement>
-- <statement> ::= <go to statement>
-- <statement> ::= <dummy statement>
-- <statement> ::= <conditional statement>
-- <statement> ::= <for statement>
-- <statement> ::= <procedure statement>
--
-- Output code generated by this routine inherits output code generated
-- by specific routine that performs actual parsing. */

static int is_declaration(void);
static CODE *block_or_compound_statement(void);

static CODE *statement(void)
{     CODE *code;
      if (t_code == T_IDENT) get_token2();
      /* recognize statement */
      if (t_delim(S_BEGIN))
      {  /* block or compound statement */
         code = block_or_compound_statement();
      }
      else if (t_code == T_IDENT && token[2].code == T_DELIM &&
         (token[2].delim == S_ASSIGN || token[2].delim == S_BEGSUB))
      {  /* assignment statement */
         code = assignment_statement(0);
      }
      else if (t_delim(S_GOTO))
      {  /* go to statement */
         code = go_to_statement();
      }
      else if (t_delim(S_ELSE) || t_delim(S_END) ||
         t_delim(S_SEMICOLON))
      {  /* dummy statement */
         code = dummy_statement();
      }
      else if (t_delim(S_IF))
      {  /* conditional statement */
         code = conditional_statement();
      }
      else if (t_delim(S_FOR))
      {  /* for statement */
         code = for_statement();
      }
      else if (t_code == T_IDENT && token[2].code == T_DELIM &&
         (token[2].delim == S_LEFT || token[2].delim == S_ELSE ||
          token[2].delim == S_END || token[2].delim == S_SEMICOLON))
      {  /* procedure statement */
         code = procedure_statement();
      }
      else if (t_delim(S_EOF))
         error("unexpected eof");
      else
      {  /* erroneous context */
         if (t_code == T_IDENT)
            error("invalid use of identifier `%s'", t_image);
         else if (t_code == T_INT || t_code == T_REAL ||
            t_code == T_FALSE || t_code == T_TRUE)
            error("invalid use of constant `%s'", t_image);
         else if (t_code == T_STRING)
            error("invalid use of string");
         else if (is_declaration())
            error("declarator `%s' in invalid position", t_image);
         else
            error("invalid use of delimiter `%s'", t_image);
         /* skip context */
         while (!(t_delim(S_EOF) || t_delim(S_ELSE) ||
            t_delim(S_END) || t_delim(S_SEMICOLON))) get_token();
      }
      /* check delimiter terminating statement */
      if (!(t_delim(S_EOF) || t_delim(S_SEMICOLON) || t_delim(S_ELSE) ||
         t_delim(S_END)))
      {  error("missing semicolon, `else', or `end' after statement");
         /* skip everything until proper delimiter */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON) ||
                  t_delim(S_ELSE) || t_delim(S_END)))
            get_token();
      }
      /* delimiter is processed NOT here */
      return code;
}

/*----------------------------------------------------------------------
-- has_labels - check whether block has lables.
--
-- This routine checks whether block b has local lables which are used
-- in any designational expressions. */

static int has_labels(BLOCK *b)
{     IDENT *id;
      assert(second_pass);
      for (id = b->first; id != NULL; id = id->next)
         if (id->flags == F_LABEL && id->ssn_used != 0) return 1;
      return 0;
}

/*----------------------------------------------------------------------
-- enter_block - enter block.
--
-- This routine translates an entry to a block.
--
-- If entered block is dummy procedure block then proc points to
-- appropriate procedure identifier, otherwise proc is NULL. In any
-- case parameter ssn is source line number where block starts.
--
-- On the first pass entered block is created and becomes current. On
-- the second pass entered block just becomes current.
--
-- Generated output code is used: (a) to save current stack top (the
-- stack used only to allocate local arrays); and (b) to initialize
-- labels for global go to. The latter is used only if entered block
-- has labels which are referenced in any designational expressions. */

static CODE *enter_block(IDENT *proc, int ssn)
{     BLOCK *b;
      CODE *code = new_code();
      if (first_pass)
      {  /* create a new block */
         b = my_malloc(sizeof(BLOCK));
         b->seqn = (last_b == NULL ? 0 : last_b->seqn + 1);
         b->ssn = ssn;
         b->proc = proc;
         b->first = b->last = NULL;
         b->surr = current;
         b->next = NULL;
         if (first_b == NULL)
            first_b = b;
         else
            last_b->next = b;
         current = last_b = b;
      }
      else
      {  /* entered block becomes current */
         if (last_b == NULL)
            last_b = first_b;
         else
            last_b = last_b->next;
         current = last_b;
         assert(current != NULL);
         /* generate output code */
         append(code, "      /* start of %s block %d (level %d) at line"
            " %d */\n", proc != NULL ? "procedure" : "local",
            current->seqn, block_level(current), current->ssn);
         append(code, "      dsa_%d->old_top_%d = " a_stack_top ";\n",
            current_level(), block_level(current));
         /* initialize labels for global go to */
         if (has_labels(current))
         {  IDENT *id;
            append(code, "      /* jmp_buf must be of array type (ISO) "
               "*/\n");
            append(code, "      switch (setjmp(&dsa_%d->jump_%d[0]))\n",
               current_level(), block_level(current));
            append(code, "      {  case 0: break;\n");
            for (id = current->first; id != NULL; id = id->next)
            {  if (!(id->flags == F_LABEL && id->ssn_used != 0))
                  continue;
               /* local labels are numbered by emit_dsa_code */
               assert(id->dim > 0);
               /* when global go to occurs then it is neccessary to
                  restore stack top and active DSA pointer for current
                  procedure */
               append(code, "         case %d: " a_pop_stack "(dsa_%d->"
                  "new_top_%d); " a_active_dsa " = (struct dsa *)dsa_%d"
                  "; goto %s_%d;\n",
                  id->dim, current_level(), block_level(current),
                  current_level(), id->name, current->seqn);
            }
            append(code, "         default: " a_fault "(\"internal erro"
               "r on global go to\");\n");
            append(code, "      }\n");
         }
      }
      return code;
}

/*----------------------------------------------------------------------
-- leave_block - leave block.
--
-- This routine translates normal exit from the current block.
--
-- On the first pass all undeclared identifiers localized in the current
-- block are moved to surround block (only if current block is not
-- outermost environmental dummy block). Thus leaved block will contain
-- no undeclared identifiers, and finally to the end of the first pass
-- all undeclared identifiers will be accumulated in the outermost block
-- that has number 0.
--
-- On the second pass the routine generates output code used to exit
-- from a block through its end bracket. This code calls pop_stack
-- library routine that restores stack top to its value before block
-- was entered freeing all memory allocated to local arrays. */

static CODE *leave_block(void)
{     BLOCK *old = current;
      CODE *code = new_code();
      assert(current != NULL);
      /* generate code to restore stack top */
      if (second_pass)
      {  append(code, "      " a_pop_stack "(dsa_%d->old_top_%d);\n",
            current_level(), block_level(current));
         append(code, "      /* end of block %d */\n", current->seqn);
      }
      /* surround block becomes the current block */
      current = current->surr;
      /* now we should move all undeclared identifiers from old current
         block to new current block (only if new current block is not
         outermost dummy block that has no parent); on the second pass
         no undeclared identifiers exist */
      if (first_pass && current != NULL)
      {  IDENT *pred, *id, *it;
         pred = NULL; id = old->first;
         while (id != NULL)
         {  if (id->flags == 0)
            {  /* remove undeclared identifier from old current block */
               if (pred == NULL)
                  old->first = id->next;
               else
                  pred->next = id->next;
               if (old->last == id) old->last = pred;
               /* and include this identifier to new current block */
               it = look_up(id->name, 0, id->ssn_used);
               if (it->dim < 0) it->dim = id->dim;
               /* free memory used by removed identifier */
               it = id, id = id->next;
               my_free(it);
            }
            else
               pred = id, id = id->next;
         }
      }
      return code;
}

/*----------------------------------------------------------------------
-- is_declaration - check for declaration.
--
-- This routine checks whether the current token begins declaration. */

static int is_declaration(void)
{     if (t_delim(S_ARRAY) || t_delim(S_BOOLEAN) ||
          t_delim(S_INTEGER) || t_delim(S_OWN) ||
          t_delim(S_PROCEDURE) || t_delim(S_REAL) ||
          t_delim(S_SWITCH)) return 1;
      return 0;
}

/*----------------------------------------------------------------------
-- block_or_compound_statement - parse block or compound statement.
--
-- This routine parses unlabelled block or compound statements using
-- the following syntax:
--
-- <block> ::= begin <declaration list> <statement list> end
-- <compound statement> ::= begin <statement list> end
-- <declaration list> ::= <declaration> ;
-- <declaration list> ::= <declaration list> <declaration> ;
-- <statement list> ::= <label list> <statement>
-- <statement list> ::= <statement list> ; <label list> <statement>
--
-- Generated output code inherits output code generated by parsing
-- routines for label list, statement, and declaration. */

static CODE *declaration(void);

static CODE *block_or_compound_statement(void)
{     CODE *code;
      int is_block;
      /* the current token must be 'begin' */
      assert(t_delim(S_BEGIN));
      get_token(/* begin */);
      /* if begin is followed by declaration then block starts */
      is_block = is_declaration();
      /* enter block and translate declaration list */
      if (is_block)
      {  code = enter_block(NULL, t_ssn);
         for (;;)
         {  if (!is_declaration()) break;
            catenate(code, declaration());
            /* semicolon following declaration is checked by the
               appropriate parsing routine */
            assert(t_delim(S_SEMICOLON) || t_delim(S_EOF));
            if (t_delim(S_SEMICOLON)) get_token(/* ; */);
         }
         /* each array declaration changes stack top and so new stack
            top should be saved (this is used when global go to leads
            to the block to restore actual value of stack top) */
         append(code, "      dsa_%d->new_top_%d = " a_stack_top ";\n",
            current_level(), block_level(current));
      }
      else
      {  /* no code is needed to enter compound statement */
         code = new_code();
      }
      /* translate list of (labelled) statements */
      for (;;)
      {  catenate(code, label_list());
         catenate(code, statement());
         if (t_delim(S_EOF))
         {  error("missing `end' bracket");
            break;
         }
         else if (t_delim(S_ELSE))
         {  error("invalid use of delimiter `else' outside if statement"
               );
            get_token(/* else */); /* ignore it */
         }
         else if (t_delim(S_END))
            break;
         else if (t_delim(S_SEMICOLON))
         {  /* semicolon means that the next statement is expected */
            get_token(/* ; */);
         }
         else
            assert(2 + 2 == 5);
      }
      /* leave the current block */
      if (is_block) catenate(code, leave_block());
      if (t_delim(S_END)) get_token(/* end */);
      return code;
}

/*----------------------------------------------------------------------
-- type_declaration - parse type declaration.
--
-- This routine parses type declaration using the syntax:
--
-- <type declaration> ::= <type> <identifier list>
-- <type declaration> ::= own <type> <identifier list>
-- <type> ::= real | integer | Boolean
-- <identifier list> ::= <identifier>
-- <identifier list> ::= <identifier list> , <identifier>
--
-- Parameter flags reflects <type> or own <type> recognized by parsing
-- routine for declaration.
--
-- Output code is generated only for own simple variables (to declare
-- and initialize them) and it is emitted immediately. */

static void type_declaration(int flags)
{     IDENT *id;
      /* the current token follows real, integer, or Boolean */
      for (;;)
      {  if (t_code != T_IDENT)
         {  error("missing simple variable identifier");
            break;
         }
         /* declare identifier of simple variable */
         id = look_up(t_image, 1, t_ssn);
         id->flags = flags;
         /* generate code for own simple variable */
         if (flags & F_OWN)
         {  if (flags & F_REAL)
               append(emit, "static double %s_%d = 0.0;\n\n",
                  id->name, id->block->seqn);
            else if (flags & F_INT)
               append(emit, "static int %s_%d = 0;\n\n",
                  id->name, id->block->seqn);
            else if (flags & F_BOOL)
               append(emit, "static bool %s_%d = false;\n\n",
                  id->name, id->block->seqn);
            else
               assert(flags != flags);
         }
         get_token(/* id */);
         /* comma following simple variable identifier means that more
            identifier is expected */
         if (!t_delim(S_COMMA)) break;
         get_token(/* , */);
      }
      /* check semicolon terminating type declaration */
      if (!t_delim(S_SEMICOLON))
      {  error("missing semicolon after type declaration");
         /* skip everything until semicolon */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
            get_token();
      }
      /* semicolon is processed NOT here */
      return;
}

/*----------------------------------------------------------------------
-- own_bound - parse bound expression for own array.
--
-- This routine parses bound expression for own array. The expression
-- must have syntactic form <integer> (i.e. optional sign followed by
-- unsigned integer).
--
-- Generated code is the same as if it were returned by parsing routine
-- for expression of general kind. */

static CODE *own_bound(void)
{     CODE *code = new_code(), *expr;
      /* check for optional sign */
      if (t_delim(S_PLUS))
         append(code, "+"), get_token(/* + */);
      else if (t_delim(S_MINUS))
         append(code, "-"), get_token(/* - */);
      /* check for integer constant */
      if (t_code == T_INT) get_token2();
      if (!(t_code == T_INT && token[2].code == T_DELIM &&
         (token[2].delim == S_COLON || token[2].delim == S_COMMA
         || token[2].delim == S_ENDSUB)))
         error("invalid bound expression for own array");
      expr = expression();
      if (second_pass) code->type = expr->type;
      catenate(code, expr);
      return code;
}

/*----------------------------------------------------------------------
-- array_declaration - parse array declaration.
--
-- This routine parses array declaration using the following syntax:
--
-- <array declaration> ::= <type> array <segment list>
-- <array declaration> ::= own <type> array <segment list>
-- <array declaration> ::= array <segment list>
-- <array declaration> ::= own array <segment list>
-- <segment list> ::= <array segment>
-- <segment list> ::= <segment list> , <array segment>
-- <array segment> ::= <identifier> [ <bound list> ]
-- <array segment> ::= <identifier> , <array segment>
-- <bound list> ::= <expression> : <expression>
-- <bound list> ::= <bound list> , <expression> : <expression>
--
-- Parameter flags reflects <type> array or own <type> array recognized
-- by parsing routine for declaration.
--
-- For last array in segment generated output code has the form:
--
-- dv = xxx_array(type, n, l1, h1, ..., ln, hn);
--
-- where xxx_array is library routine alloc_array (for local array) or
-- own_array (for own array); type is 'r', 'i', or 'b' indicating type
-- of array; n - number of bound pairs (i.e. number of subscripts);
-- l1, h1, ..., ln, hn are expressions for lower and upper bounds for
-- array.
--
-- For not last array in segment generated output code has the form:
--
-- dv = xxx_same(type, proto_dv);
--
-- where xxx_same is library routine alloc_same (for local array) or
-- own_same (for own array); proto_dv is dope vector for array from the
-- same array segment that has been allocated already (it is used to
-- get dimension informataion for array to be allocated).
--
-- When local array is allocated then allocation memory (for dope vector
-- and for array components) is pushed into stack by library routine
-- alloc_array or same_array. This memory will be automatically freed
-- for each block that is leaved through its end bracket or as a result
-- of global go to by library routine pop_stack. Each own array is
-- allocated only once on the first entrance to the appropriate block by
-- library routine own_array or own_same and never been freed. */

static CODE *array_declaration(int flags)
{     CODE *code = new_code();
      IDENT *id[1+100]; /* current array segment */
      int n, dim;
      char *place;
      /* the current token follows 'array' */
      /* set flag to check clause 5.2.4.2 (see Modified Report) */
      array_decl_flag = 1;
loop: /* parse the current array segment */
      for (n = 1; ; n++)
      {  if (t_code != T_IDENT)
         {  error("missing array identifier");
            goto skip;
         }
         if (n > 100)
         {  error("too many identifiers in array segment");
            n = 1;
         }
         /* declare identifier of array */
         id[n] = look_up(t_image, 1, t_ssn);
         id[n]->flags = flags;
         get_token(/* id */);
         /* emit code to declare dope vector for own array */
         if (flags & F_OWN)
            append(emit, "static struct dv *%s_%d = NULL;\n\n",
               id[n]->name, id[n]->block->seqn);
         if (!t_delim(S_COMMA)) break;
         get_token(/* , */);
      }
      /* check left parenthesis */
      if (!t_delim(S_BEGSUB))
      {  error("missing left parenthesis after array segment");
         goto skip;
      }
      /* generate code to allocate last array in segment */
      catenate(code, emit_ssn(id[n]->ssn_decl));
      if (flags & F_OWN)
         append(code, "      if (%s_%d == NULL) %s_%d = " a_own_array,
            id[n]->name, id[n]->block->seqn,
            id[n]->name, id[n]->block->seqn);
      else
         append(code, "      dsa_%d->%s_%d = " a_alloc_array,
            current_level(), id[n]->name, id[n]->block->seqn);
      append(code, "('%s', ?, ",
         (flags & F_REAL) ? "r" : (flags & F_INT) ? "i" : "b");
      if (second_pass)
      {  /* remember place to substitute array dimension */
         place = strchr(code->tail->str, '?');
         assert(place != NULL);
      }
      /* translate bound list and determine dimension of array */
      get_token(/* [ */);
      dim = 0;
      for (;;)
      {  CODE *bound;
         if (dim == 9)
         {  error("array dimension exceeds allowable maximum");
            dim = 0;
         }
         dim++;
         bound = flags & F_OWN ? own_bound() : expression();
         if (second_pass)
         {  if (bound->type == F_REAL) to_int(bound);
            if (bound->type != F_INT)
            {  error("bound expression is not of arithmetic type");
               bound->type = F_INT;
            }
            catenate(code, bound);
            append(code, ", ");
         }
         if (!t_delim(S_COLON))
         {  error("missing colon between bound expressions");
            goto skip;
         }
         get_token(/* : */);
         bound = flags & F_OWN ? own_bound() : expression();
         if (second_pass)
         {  if (bound->type == F_REAL) to_int(bound);
            if (bound->type != F_INT)
            {  error("bound expression is not of arithmetic type");
               bound->type = F_INT;
            }
            catenate(code, bound);
            append(code, t_delim(S_COMMA) ? ", " : ");\n");
         }
         if (!t_delim(S_COMMA)) break;
         get_token(/* , */);
      }
      if (!t_delim(S_ENDSUB))
      {  error("missing right parenthesis after bound list");
         goto skip;
      }
      get_token(/* ] */);
      /* now dimension of array is known */
      {  int k;
         assert(1 <= dim && dim <= 9);
         for (k = 1; k <= n; k++) id[k]->dim = dim;
         if (second_pass) *place = (char)(dim + '0');
      }
      /* generate code for the other arrays in segment */
      for (;;)
      {  if (--n < 1) break;
         catenate(code, emit_ssn(id[n]->ssn_decl));
         if (flags & F_OWN)
            append(code, "      if (%s_%d == NULL) %s_%d = " a_own_same,
               id[n]->name, id[n]->block->seqn,
               id[n]->name, id[n]->block->seqn);
         else
            append(code, "      dsa_%d->%s_%d = " a_alloc_same,
               current_level(), id[n]->name, id[n]->block->seqn);
         append(code, "('%s', ",
            (flags & F_REAL) ? "r" : (flags & F_INT) ? "i" : "b");
         if (flags & F_OWN)
            append(code, "%s_%d);\n",
               id[n+1]->name, id[n+1]->block->seqn);
         else
            append(code, "dsa_%d->%s_%d);\n",
               current_level(), id[n+1]->name, id[n+1]->block->seqn);
      }
      /* if the current array segment is followed by comma then more
         array segment is expected */
      if (t_delim(S_COMMA))
      {  get_token(/* , */);
         goto loop;
      }
      /* clear array declaration flag */
      array_decl_flag = 0;
      /* check semicolon terminating array declaration */
      if (!t_delim(S_SEMICOLON))
      {  error("missing semicolon after array declaration");
skip:    /* skip everything until semicolon */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
            get_token();
      }
      /* semicolon is processed NOT here */
      return code;
}

/*----------------------------------------------------------------------
-- switch_declaration - parse switch declaration.
--
-- This routine parses switch declaration using the syntax:
--
-- <switch declaration> ::= switch <identifier> := <switch list>
-- <switch list> ::= <expression>
-- <switch list> ::= <switch list> , <expression>
--
-- Since switch may be passed as actual parameter switch declaration is
-- always translated to separate static routine. That routine uses its
-- parameter (value of subscript expression from switch designator) to
-- evaluate struct label representing "value of label", i.e. value of
-- the corresponding designational expression. Generated output code is
-- immediately emitted to final output code. */

static void switch_declaration(void)
{     IDENT *id;
      int dim;
      /* the current token follows 'switch' */
      if (t_code != T_IDENT)
      {  error("missing switch identifier");
         goto skip;
      }
      /* declare identifier of switch */
      id = look_up(t_image, 1, t_ssn);
      id->flags = F_SWITCH;
      get_token(/* id */);
      if (!t_delim(S_ASSIGN))
      {  error("missing `:=' after switch identifier");
         goto skip;
      }
      get_token(/* := */);
      /* generate and emit the head of output code */
      append(emit, "static struct label %s_%d(int kase)\n",
         id->name, id->block->seqn);
      append(emit, "{     /* switch declaration at line %d */\n",
         id->ssn_decl);
      emit_dsa_pointers();
      catenate(emit, emit_ssn(id->ssn_decl));
      append(emit, "      switch (kase)\n");
      /* translate members of switch list */
      dim = 0;
      for (;;)
      {  CODE *expr = expression();
         dim++;
         if (second_pass && expr->type != F_LABEL)
         {  error("expression in switch list is not of label type");
            expr->type = F_LABEL;
         }
         append(emit, "      %s  case %d: dsa_%d->line = %d; return ",
            dim == 1 ? "{" : " ", dim, current_level(), t_ssn);
         catenate(emit, expr);
         append(emit, ";\n");
         /* if member of switch list is followed by comma then more
            designational expression is expected */
         if (!t_delim(S_COMMA)) break;
         get_token(/* , */);
      }
      /* generate and emit the tail of output code */
      append(emit, "         default: " a_fault "(\"switch designator u"
         "ndefined\");\n");
      append(emit, "      }\n");
      append(emit, "      return " a_make_label "(NULL, 0);\n");
      append(emit, "}\n\n");
      /* check semicolon terminating switch declaration */
      if (!t_delim(S_SEMICOLON))
      {  error("missing semicolon after switch declaration");
skip:    /* skip everything until semicolon */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
            get_token();
      }
      /* semicolon is processed NOT here */
      return;
}

/*----------------------------------------------------------------------
-- emit_proc_head - generate and emit code for procedure heading.
--
-- This routine generates and emits output code for procedure heading.
-- Parameter proc points to procedure identifier for which this code is
-- generated. If parameter flag is clear then generated code is used
-- as heading of translated routine; otherwise, if flag is set, then
-- generated code is prototype of translated routine (the latter is used
-- to declare translated routine before its usage since its delaration
-- may appear after its usage). */

static void emit_proc_head(IDENT *proc, int flag)
{     /* if flag = 0, then 'type func(...) { ... }' is generated
         if flag = 1, then 'type func(...);' is generated */
      BLOCK *b;
      IDENT *id;
      if (first_pass) goto skip;
      /* pseudo procedures inline and print need no output code */
      if ((proc->flags & F_BLTIN) &&
          (strcmp(proc->name, "inline") == 0 ||
           strcmp(proc->name, "print")  == 0)) goto skip;
      /* search for procedure dummy block that holds formal parameters
         of specified procedure */
      for (b = first_b; b != NULL; b = b->next)
         if (b->proc == proc) break;
      assert(b != NULL);
      /* generate and emit output code for procedure itself */
      if (flag)
      {  if (proc->block->seqn == 0)
            append(emit, "extern ");
         else
            append(emit, "static ");
      }
      append(emit, "struct desc %s_%d", proc->name, proc->block->seqn);
      if (strcmp(proc->name, "main_program") == 0)
         append(emit, " /* program */");
      else
         append(emit, " /* %s %s procedure */",
            proc->flags & F_CODE   ? "code" :
            proc->flags & F_BLTIN  ? "builtin" :
            proc->block->seqn == 0 ? "precompiled" : "local",
            proc->flags & F_REAL   ? "real" :
            proc->flags & F_INT    ? "integer" :
            proc->flags & F_BOOL   ? "Boolean" : "void");
      if (proc->dim == 0)
      {  append(emit, " (void)");
         goto done;
      }
      /* generate and emit output code for formal parameters */
      append(emit, "\n");
      for (id = b->first; id != NULL; id = id->next)
      {  append(emit, "%s     struct arg ", id == b->first ? "(" : " ");
         if (flag)
            append(emit, "/* %s:", id->name);
         else
            append(emit, "%s_%d /*", id->name, b->seqn);
         if (id->flags & F_BYVAL)   append(emit, " by value");
         if (id->flags & F_BYNAME)  append(emit, " by name");
         if (id->flags & F_REAL)    append(emit, " real");
         if (id->flags & F_INT)     append(emit, " integer");
         if (id->flags & F_BOOL)    append(emit, " Boolean");
         if (id->flags & F_LABEL)   append(emit, " label");
         if (id->flags & F_ARRAY)   append(emit, " array");
         if (id->flags & F_SWITCH)  append(emit, " switch");
         if (id->flags & F_PROC)    append(emit, " procedure");
         if (id->flags & F_STRING)  append(emit, " string");
         append(emit, " */%s", id->next == NULL ? "\n)" : ",\n");
      }
done: append(emit, flag ? ";\n\n" : "\n");
skip: return;
}

/*----------------------------------------------------------------------
-- procedure_declaration - parse procedure declaration.
--
-- This routine parses procedure declaration using the syntax:
--
-- <procedure declaration> ::= procedure <procedure heading>
--                             <procedure body>
-- <procedure declaration> ::= <type> procedure <procedure heading>
--                             <procedure body>
-- <type> ::= real | integer | Boolean
-- <procedure heading> ::= <identifier> <formal parameter part> ;
--                         <value part> <specification part>
-- <formal parameter part> ::= <empty>
-- <formal parameter part> ::= ( <identifier list> )
-- <value part> ::= <empty>
-- <value part> ::= value <identifier list>
-- <specification part> ::= <empty>
-- <specification part> ::= <specifier> <identifier list> ;
-- <specification part> ::= <specification part> <specifier>
--                          <identifier list> ;
-- <specifier> ::= string | <type> | <type> array | array | label |
--                 switch | procedure | <type> procedure
-- <type> ::= real | integer | Boolean
-- <identifier list> ::= <identifier>
-- <identifier list> ::= <identifier list> , <identifier>
-- <procedure body> ::= <label list> <statement>
-- <procedure body> ::= code
--
-- Note that <program> is treated as if it where <procedure declaration>
-- with implicit heading 'procedure main_program;'.
--
-- Parameter flags reflects optional <type> of procedure recognized by
-- parsing routine for declaration.
--
-- If procedure body is a statement then procedure declaration is
-- translated to actual C routine. If procedure body is a key word code
-- then no translation is performed because only prototype is used.
--
-- Generated code is immediately emitted to final output code and is
-- used: (a) to enter dummy procedure block; (b) to evaluate all formal
-- parameters called by value; (c) to enter dummy block that encloses
-- procedure body; (d) to execute statement that represents procedure
-- body; (e) to leave dummy block that encloses procedure body; (f) to
-- leave dummy procedure block and return optional value assigned to
-- procedure identifier in the form of struct desc.
--
-- Each procedure declared inside a block is translated with attribute
-- 'static' because it is seen only inside the appropriate block. But
-- each procedure declared outside any block (i.e. in the environmental
-- block) is translated without attribute 'static' because it is may be
-- called from the other module as a precompiled procedure. */

static void procedure_declaration(int flags)
{     CODE *code, *prolog;
      IDENT *proc; /* procedure identifier */
      int is_main; /* main program flag */
      int dim; /* number of formal parameters */
      if (token[0].code == T_DELIM && token[0].delim == S_PROCEDURE)
      {  /* procedure declaration will be parsed */
         is_main = 0;
      }
      else
      {  /* main program will be parsed */
         is_main = 1;
      }
      /* declare procedure identifier or dummy identifier in the case
         of main program */
      if (!is_main)
      {  if (t_code == T_IDENT)
         {  proc = look_up(t_image, 1, t_ssn);
            get_token(/* id */);
         }
         else
         {  char str[50];
            sprintf(str, "p_%d", t_ssn);
            error("missing procedure identifier after `procedure'; dumm"
               "y identifier `%s' used", str);
            proc = look_up(str, 1, t_ssn);
         }
      }
      else
      {  assert(current->seqn == 0);
         proc = look_up("main_program", 1, t_ssn);
      }
      proc->flags = flags;
      /* enter dummy procedure block that will hold formal parameters
         from procedure heading */
      prolog = enter_block(proc, t_ssn);
      /* since main program is syntactically equivalent to procedure
         body, procedure heading should not be processed */
      dim = 0;
      if (is_main) goto skip;
      /* process formal parameter list */
      if (t_delim(S_LEFT))
      {  get_token(/* ( */);
         for (;;)
         {  if (t_code != T_IDENT)
            {  error("missing formal parameter identifier");
               break;
            }
            /* add identifier to dummy procedure block */
            if (first_pass)
            {  IDENT *id = look_up(t_image, 0, t_ssn);
               if (id->flags & F_BYNAME)
                  error("formal parameter `%s' repeated in formal param"
                     "eter list", id->name);
               id->flags = F_BYNAME;
               /* formal parameter list should not contain the procedure
                  identifier from the same procedure heading (see clause
                  5.4.3 of the Modified Report) */
               if (strcmp(id->name, proc->name) == 0)
                  error("formal parameter identifier `%s' is the same a"
                     "s procedure identifier", id->name);
            }
            dim++;
            get_token(/* id */);
            /* extended parameter delimiter following formal parameter
               identifier means that more identifier is expected */
            if (!ext_comma()) break;
         }
         /* check closing parenthesis */
         if (t_delim(S_RIGHT))
            get_token(/* ) */);
         else
            error("missing right parenthesis after formal parameter lis"
               "t");
      }
      /* check semicolon */
      if (!t_delim(S_SEMICOLON))
      {  error("missing semicolon after formal parameter part");
         /* skip everything until semicolon */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
            get_token();
      }
      get_token(/* ; */);
      /* process optional value part */
      if (t_delim(S_VALUE))
valp: {  for (;;)
         {  get_token(/* value or , */);
            if (t_code != T_IDENT)
            {  error("missing formal parameter identifier");
               break;
            }
            if (first_pass)
            {  IDENT *id = look_up(t_image, 0, t_ssn);
               if (!id->flags)
                  error("identifier `%s' missing from formal parameter "
                     "list", id->name);
               if (id->flags & F_BYVAL)
                  error("formal parameter `%s' repeated in value part",
                     id->name);
               id->flags = F_BYVAL;
            }
            get_token(/* id */);
            /* comma following formal parameter identifier means that
               more identifier is expected */
            if (!t_delim(S_COMMA)) break;
         }
         /* check semicolon terminating value part */
         if (!t_delim(S_SEMICOLON))
         {  error("missing semicolon after value part");
            /* skip everything until semicolon */
            while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
               get_token();
         }
         get_token(/* ; */);
      }
      /* process optional specification part */
      while (t_delim(S_ARRAY) || t_delim(S_BOOLEAN) ||
         t_delim(S_INTEGER) || t_delim(S_LABEL) ||
         t_delim(S_PROCEDURE) || t_delim(S_REAL) ||
         t_delim(S_STRING) || t_delim(S_SWITCH))
      {  int flags;
         if (t_delim(S_REAL) || t_delim(S_INTEGER) ||
            t_delim(S_BOOLEAN))
         {  /* simple parameter, array, or procedure */
            flags = t_delim(S_REAL) ? F_REAL : t_delim(S_INTEGER) ?
               F_INT : F_BOOL;
            get_token(/* real, inteter, Boolean */);
            if (t_delim(S_ARRAY))
               flags |= F_ARRAY, get_token(/* array */);
            else if (t_delim(S_PROCEDURE))
               flags |= F_PROC, get_token(/* procedure */);
         }
         else if (t_delim(S_LABEL))
            flags = F_LABEL, get_token(/* label */);
         else if (t_delim(S_ARRAY))
            flags = F_REAL | F_ARRAY, get_token(/* array */);
         else if (t_delim(S_SWITCH))
            flags = F_SWITCH, get_token(/* switch */);
         else if (t_delim(S_PROCEDURE))
            flags = F_PROC, get_token(/* procedure */);
         else if (t_delim(S_STRING))
            flags = F_STRING, get_token(/* string */);
         else
            assert(2 + 2 == 5);
         /* process identifier list */
         for (;;)
         {  if (t_code != T_IDENT)
            {  error("missing formal parameter identifier");
               break;
            }
            if (first_pass)
            {  IDENT *id = look_up(t_image, 0, t_ssn);
               if (!id->flags)
                  error("identifier `%s' missing from formal parameter "
                     "list", id->name);
               if (id->flags & ~(F_BYNAME | F_BYVAL))
                  error("formal parameter `%s' multiply specified",
                     id->name);
               /* specification is like declaration */
               id->ssn_decl = t_ssn;
               id->ssn_used = 0;
               id->flags |= flags;
               /* additional checking for specification */
               if ((id->flags & F_BYVAL) &&
                   (id->flags & (F_SWITCH | F_PROC | F_STRING)))
                  error("invalid call by value of switch, procedure, or"
                     " string `%s'", id->name);
            }
            get_token(/* id */);
            /* comma following formal parameter identifier means that
               more identifier is expected */
            if (!t_delim(S_COMMA)) break;
            get_token(/* , */);
         }
         /* check semicolon terminating specification */
         if (!t_delim(S_SEMICOLON))
         {  error("missing semicolon after specification");
            /* skip everything until semicolon */
            while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
               get_token();
         }
         get_token(/* ; */);
      }
      /* check for often error */
      if (t_delim(S_VALUE))
      {  error("specification part precedes value part");
         goto valp;
      }
      /* all formal parameters must be specified */
      {  IDENT *id;
         int ok = 1;
         for (id = current->first; id != NULL; id = id->next)
         {  if (!(id->flags & ~(F_BYNAME | F_BYVAL)))
            {  error("formal parameter `%s' not specified", id->name);
               ok = 0;
            }
         }
         if (!ok)
            error("specification part of procedure `%s' incomplete",
               proc->name);
      }
skip: /* now the number of formal parameters is known */
      proc->dim = dim;
      /* translate procedure body */
      /* if procedure body is code then only prototype is needed which
         has been generated already; nevertheless we should leave dummy
         procedure block ignoring output code */
      if (t_delim(S_CODE))
      {  assert(!is_main);
         if (current->surr->seqn != 0)
            error("invalid declaration of code procedure inside block");
         proc->flags |= F_CODE; /* set code procedure flag */
         free_code(prolog);
         free_code(leave_block());
         get_token(/* code */);
         if (!t_delim(S_SEMICOLON))
         {  error("missing semicolon after 'code'");
            /* skip everything until semicolon */
            while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
               get_token();
         }
         /* semicolon is processed NOT here */
         goto done;
      }
      /* generated code for internal procedures should precede code for
         surround procedure, ergo we can't emit generated code now */
      code = new_code();
      if (second_pass)
      {  /* generate code to enter procedure (code for procedure heading
            will be generated later) */
         /* declare DSA for the current procedure */
         append(code, "{     struct dsa_%s_%d my_dsa;\n",
            proc->name, proc->block->seqn);
         /* declare and initialize DSA pointers (using global display
            pointer global_dsa) for all surround procedures including a
            pointer to DSA for the current procedure (level of the
            current procedure is equal to one plus level of the surround
            procedure) */
         {  int level = dsa_level(proc) + 1;
            BLOCK *b;
            for (b = current; b; b = b->surr)
            {  if (b->proc == NULL) continue;
                append(code,
                  "      register struct dsa_%s_%d *dsa_%d = ",
                  b->proc->name, b->proc->block->seqn, level);
               if (b->proc == proc)
                  append(code, "&my_dsa;\n");
               else
                  append(code, "(void *)" a_global_dsa "->vector[%d];\n"
                     , level);
               level--;
            }
         }
         /* initialize standard DSA fields */
         append(code, "      my_dsa.proc = \"%s\";\n", proc->name);
         {  char str[100+10], *t;
            int k;
            for (k = 0, t = infilename; k < 100 && *t; t++)
            {  if (*t == '\\' || *t == '\"') str[k++] = '\\';
               str[k++] = *t;
            }
            str[k] = '\0';
            if (*t) strcat(str, "...");
            append(code, "      my_dsa.file = \"%s\";\n", str);
         }
         append(code, "      my_dsa.line = %d;\n", proc->ssn_decl);
         append(code, "      my_dsa.parent = " a_active_dsa ", "
            a_active_dsa " = (struct dsa *)&my_dsa;\n");
         /* use DSA pointers to fill procedure display for the current
            procedure (this display is placed in DSA) */
         {  int level = dsa_level(proc) + 1, k;
            for (k = 0; k <= level; k++)
            append(code, "      my_dsa.vector[%d] = (void *)dsa_%d;\n",
               k, k);
         }
         /* inherit code to enter dummy procedure block */
         catenate(code, prolog);
      }
      /* generate code to copy formal parameters (this is a reasonable
         place to put code for checking actual-formal correspondence);
         if formal parameter is called by value then it is evaluated as
         if it were used in expression and its value is stored in DSA
         as usual local simple variable; if formal parameter is called
         by name then the corresponding struct arg is stored in DSA;
         it is not concerned to arrays in which case only pointer to
         dope vector is stored in DSA */
      if (second_pass)
      {  IDENT *id;
         /* all formal parameters are held by dummy procedure block */
         for (id = current->first; id != NULL; id = id->next)
         {  switch (id->flags)
            {  case F_REAL | F_BYVAL:
                  /* real simple parameter called by value */
               case F_INT | F_BYVAL:
                  /* integer simple parameter called by value */
               case F_BOOL | F_BYVAL:
                  /* Boolean simple parameter called by value */
               case F_LABEL | F_BYVAL:
                  /* formal label called by value */
                  append(code, "      my_dsa.line = %d;\n",
                     id->ssn_decl);
                  append(code, "      my_dsa.%s_%d = %s((" a_global_dsa
                     " = %s_%d.arg2, (*(struct desc (*)(void))%s_%d.arg"
                     "1)()));\n", id->name, current->seqn,
                     (id->flags & F_REAL)  ? "get_real"  :
                     (id->flags & F_INT)   ? "get_int"   :
                     (id->flags & F_BOOL)  ? "get_bool"  :
                     (id->flags & F_LABEL) ? "get_label" : "???",
                     id->name, current->seqn,
                     id->name, current->seqn);
                  break;
               case F_REAL | F_ARRAY | F_BYVAL:
                  /* real formal array called by value */
               case F_INT | F_ARRAY | F_BYVAL:
                  /* integer formal array called by value */
               case F_BOOL | F_ARRAY | F_BYVAL:
                  /* Boolean formal array called by value */
                  append(code, "      my_dsa.line = %d;\n",
                     id->ssn_decl);
                  append(code, "      my_dsa.%s_%d = %s(%s_%d);\n",
                     id->name, current->seqn,
                     (id->flags & F_REAL) ? a_copy_real :
                     (id->flags & F_INT)  ? a_copy_int  :
                     (id->flags & F_BOOL) ? a_copy_bool : "???",
                     id->name, current->seqn);
                  break;
               case F_REAL | F_BYNAME:
                  /* real simple parameter called by name */
               case F_INT | F_BYNAME:
                  /* integer simple parameter called by name */
               case F_BOOL | F_BYNAME:
                  /* Boolean simple parameter called by name */
               case F_LABEL | F_BYNAME:
                  /* formal label called by name */
               case F_SWITCH | F_BYNAME:
                  /* formal swiych */
               case F_REAL | F_PROC | F_BYNAME:
                  /* formal real procedure */
               case F_INT | F_PROC | F_BYNAME:
                  /* formal integer procedure */
               case F_BOOL | F_PROC | F_BYNAME:
                  /* formal Boolean procedure */
               case F_PROC | F_BYNAME:
                  /* formal typeless procedure */
                  append(code, "      my_dsa.%s_%d = %s_%d;\n",
                     id->name, current->seqn, id->name, current->seqn);
                  break;
               case F_REAL | F_ARRAY | F_BYNAME:
                  /* formal real array called by name */
               case F_INT | F_ARRAY | F_BYNAME:
                  /* formal integer array called by name */
               case F_BOOL | F_ARRAY | F_BYNAME:
                  /* formal Boolean array called by name */
               case F_STRING | F_BYNAME:
                  /* string */
                  append(code, "      my_dsa.%s_%d = %s_%d.arg1;\n",
                     id->name, current->seqn, id->name, current->seqn);
                  break;
               default:
                  assert(id->flags != id->flags);
            }
         }
         /* generate code to save new stack top (stack top is changed
            due to copy formal array that are called by value) */
         append(code, "      dsa_%d->new_top_%d = " a_stack_top ";\n",
            current_level(), block_level(current));
      }
      /* translate procedure body; a body is always enclosed to dummy
         block (from clause 5.4.3 of the Modified Report) because the
         body may be a statement containing label declarations */
      catenate(code, enter_block(NULL, t_ssn));
      append(code, "      dsa_%d->new_top_%d = " a_stack_top ";\n",
         current_level(), block_level(current));
      catenate(code, label_list());
      /* in the case of main program label list must be followed by
         block or compound statement */
      if (is_main && !t_delim(S_BEGIN))
         error("missing bracket 'begin'");
      catenate(code, statement());
      catenate(code, leave_block());
      /* generate code to leave dummy procedure block */
      catenate(code, leave_block());
      /* generate code to return from the current procedure */
      if (second_pass)
      {  append(code, "      my_dsa.retval.lval = 0;\n");
         switch (proc->flags & (F_REAL | F_INT | F_BOOL))
         {  case F_REAL:
               append(code, "      my_dsa.retval.type = 'r';\n");
               break;
            case F_INT:
               append(code, "      my_dsa.retval.type = 'i';\n");
               break;
            case F_BOOL:
               append(code, "      my_dsa.retval.type = 'b';\n");
               break;
            default: /* typeless procedure */
               append(code, "      my_dsa.retval.type = 0;\n");
               break;
         }
         append(code, "      " a_active_dsa " = my_dsa.parent;\n");
         append(code, "      return my_dsa.retval;\n");
         append(code, "}\n\n");
      }
      /* since procedure declaration has been processed, it is possible
         to emit generated code to final output code */
      if (second_pass)
      {  emit_proc_head(proc, 0);
         catenate(emit, code);
      }
      /* check semicolon that must follow procedure declaration (in
         the case of main program semicolon is optional) */
      if (!is_main && !t_delim(S_SEMICOLON))
      {  error("missing semicolon after procedure declaration");
         /* skip everything until semicolon */
         while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
            get_token();
      }
      if (is_main && t_delim(S_SEMICOLON))
         warning("semicolon found after program");
      /* semicolon is processed NOT here */
done: return;
}

/*----------------------------------------------------------------------
-- declaration - parse declaration.
--
-- This routine parses declaration using the following syntax:
--
-- <declaration> ::= <type declaration>
-- <declaration> ::= <array declaration>
-- <declaration> ::= <switch declaration>
-- <declaration> ::= <procedure declaration>
--
-- Output code generated by this routine inherits output code generated
-- by specific routine that performs actual parsing. */

static CODE *declaration(void)
{     CODE *code;
      int flags;
      /* the current token must be declarator */
      assert(is_declaration());
      if (t_delim(S_REAL) || t_delim(S_INTEGER) || t_delim(S_BOOLEAN))
      {  flags = t_delim(S_REAL) ? F_REAL : t_delim(S_INTEGER) ? F_INT :
            F_BOOL;
         get_token(/* real, integer, Boolean */);
         if (t_delim(S_ARRAY))
            flags |= F_ARRAY, get_token(/* array */);
         else if (t_delim(S_PROCEDURE))
            flags |= F_PROC, get_token(/* procedure */);
      }
      else if (t_delim(S_ARRAY))
         flags = F_REAL | F_ARRAY, get_token(/* array */);
      else if (t_delim(S_OWN))
      {  flags = F_OWN, get_token(/* own */);
         if (t_delim(S_REAL))
            flags |= F_REAL, get_token(/* real */);
         else if (t_delim(S_INTEGER))
            flags |= F_INT, get_token(/* integer */);
         else if (t_delim(S_BOOLEAN))
            flags |= F_BOOL, get_token(/* Boolean */);
         if (t_delim(S_ARRAY))
         {  if (flags == F_OWN) flags |= F_REAL;
            flags |= F_ARRAY, get_token(/* array */);
         }
         if (flags == F_OWN)
         {  error("missing declarator after 'own'");
            flags |= F_REAL;
         }
      }
      else if (t_delim(S_SWITCH))
         flags = F_SWITCH, get_token(/* switch */);
      else if (t_delim(S_PROCEDURE))
         flags = F_PROC, get_token(/* procedure */);
      else
         assert(2 + 2 == 5);
      /* parse the corresponding declaration */
      if (flags & F_ARRAY)
         code = array_declaration(flags);
      else if (flags & F_SWITCH)
         switch_declaration(), code = new_code();
      else if (flags & F_PROC)
         procedure_declaration(flags), code = new_code();
      else
         type_declaration(flags), code = new_code();
      return code;
}

/*----------------------------------------------------------------------
-- resolving - resolve external references.
--
-- This routine processes each undeclared identifier (after the first
-- pass all undeclared identifiers are accumulated in outermost dummy
-- block that has number 0). If undeclared identifier is identifier of
-- standard procedure (including pseudo procedures inline and print),
-- then this identifier is made declared as if there were the procedure
-- declaration. Otherwise if undeclared identifier does not match any
-- standard procedure identifier then error is raised. */

static void resolving(void)
{     IDENT *id, *arg;
      assert(first_pass);
      for (id = first_b->first; id != NULL; id = id->next)
      {  if (id->flags)
            /* identifier has been declared */;
         else if (strcmp(id->name, "abs") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "iabs") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_INT | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "sign") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_INT | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "entier") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_INT | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "sqrt") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "sin") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "cos") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "arctan") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "ln") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "exp") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("E", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "stop") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 0;
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "fault") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("str", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            arg = look_up("r", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "inchar") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 3;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("str", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            arg = look_up("int", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYNAME;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "outchar") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 3;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("str", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            arg = look_up("int", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "length") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_INT | F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("str", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "outstring") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("str", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "outterminator") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "ininteger") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("int", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYNAME;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "outinteger") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("int", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "inreal") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("re", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYNAME;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "outreal") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 2;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("channel", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_INT | F_BYVAL;
            arg = look_up("re", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_REAL | F_BYVAL;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "maxreal") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 0;
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "minreal") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 0;
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "maxint") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_INT | F_PROC | F_BLTIN;
            id->dim = 0;
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
         }
         else if (strcmp(id->name, "epsilon") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_REAL | F_PROC | F_BLTIN;
            id->dim = 0;
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
         }
         /* further standard functions and procedures may be added
            here, but no additional ones may be regarded as part of the
            reference language */
         else if (strcmp(id->name, "inline") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 1;
            free_code(enter_block(id, 1));
            current->proc = id;
            arg = look_up("statement", 0, 0);
            arg->ssn_decl = arg->ssn_used = 1;
            arg->flags = F_STRING | F_BYNAME;
            free_code(leave_block());
            warning("pseudo procedure `inline' used");
         }
         else if (strcmp(id->name, "print") == 0)
         {  id->ssn_decl = 0;
            id->flags = F_PROC | F_BLTIN;
            id->dim = 0; /* special ! */
            free_code(enter_block(id, 1));
            current->proc = id;
            free_code(leave_block());
            warning("pseudo procedure `print' used");
         }
         else
            error("identifier `%s' not declared (see line %d)",
               id->name, id->ssn_used);
      }
      return;
}

/*----------------------------------------------------------------------
-- entire_module - parse module.
--
-- This routine parses module (translation unit) using the syntax:
--
-- <module> ::= <unit>
-- <module> ::= <module> <unit>
-- <unit> ::= <procedure declaration> ;
-- <unit> ::= <program> ;
-- <program> ::= <label list> <block>
-- <program> ::= <label list> <compound statement>
--
-- Note that module can be consider as the environmental block, and any
-- procedure declared in module is treated as pre-compiled procedure
-- (or code procedure if procedure body is code, but not a statement).
--
-- Generated output code inherits code generated by appropriate parsing
-- routines and is emitted to final output code. */

static int entire_module(void)
{     int is_main = 0; /* set if main program was detected */
      /* enter the first outermost dummy block that holds declarations
         of all external procedures (including main program) */
      free_code(enter_block(NULL, 0));
      /* read the very beginning token */
      l_count = 0;
      if (first_pass) line = my_malloc(l_maxlen+1);
      line[0] = '\0';
      read_line();
      pos = 0;
      symbol = S_EOF;
      s_char = '?';
      scan_symbol();
      token[0].ssn = 0;
      token[0].code = T_DELIM;
      token[0].delim = S_EOF;
      token[0].len = 0;
      if (first_pass) token[0].image = my_malloc(t_maxlen+1);
      token[0].image[0] = '\0';
      token[1].ssn = 0;
      token[1].code = T_DELIM;
      token[1].delim = S_EOF;
      token[1].len = 0;
      if (first_pass) token[1].image = my_malloc(t_maxlen+1);
      token[1].image[0] = '\0';
      token[2].ssn = 0;
      token[2].code = T_UNDEF;
      token[2].delim = S_EOF;
      token[2].len = 0;
      if (first_pass) token[2].image = my_malloc(t_maxlen+1);
      token[2].image[0] = '\0';
      get_token();
      /* check for null program */
      if (token[1].code == T_DELIM && token[1].delim == S_EOF)
      {  error("null program not allowed");
         goto skip;
      }
      /* main loop */
      for (;;)
      {  int flags;
         /* check for end of input text file */
         if (t_delim(S_EOF)) break;
         /* the current unit may be only labelled block or compound
            statement representing main program, or declaration of
            (pre-compiled or code) procedure */
         if (t_code == T_IDENT) get_token2();
         if (t_delim(S_BEGIN) ||
            (t_code == T_IDENT &&
             token[2].code == T_DELIM && token[2].delim == S_COLON))
         {  if (is_main)
               error("only one program allowed");
            is_main = 1, flags = F_PROC;
         }
         else
         {  if (t_delim(S_REAL))
               flags = F_REAL, get_token(/* real */);
            else if (t_delim(S_INTEGER))
               flags = F_INT, get_token(/* integer */);
            else if (t_delim(S_BOOLEAN))
               flags = F_BOOL, get_token(/* Boolean */);
            else
               flags = 0;
            if (t_delim(S_PROCEDURE))
               flags |= F_PROC, get_token(/* procedure */);
         }
         if (!(flags & F_PROC))
         {  error("invalid start of program or precompiled procedure");
err:        /* ignore erroneous context */
            while (!(t_delim(S_EOF) || t_delim(S_SEMICOLON)))
               get_token();
            if (t_delim(S_SEMICOLON)) get_token(/* ; */);
            continue;
         }
         /* parse procedure declaration (including main program which
            is considered as procedure that has no heading) */
         procedure_declaration(flags);
         /* procedure declaration is always followed by semicolon
            (checked by the appropriate parsing routine); but in the
            case of <program> semicolon is optional and should be used
            if others procedure declarations follow <program> */
         if (t_delim(S_SEMICOLON))
            get_token(/* ; */);
         else if (!t_delim(S_EOF))
         {  error("equal number of 'begin' and 'end' brackets found");
            goto err;
         }
      }
skip: /* resolve external references */
      if (first_pass) resolving();
      /* leave the outermost dummy block ignoring code */
      free_code(leave_block());
      assert(current == NULL);
      return is_main;
}

/*----------------------------------------------------------------------
-- proc_block - search for procedure block enclosing given block.
--
-- This routine returns pointer to procedure block that encloses given
-- block b. */

static BLOCK *proc_block(BLOCK *b)
{     for (; b != NULL; b = b->surr)
         if (b->proc != NULL) break;
      return b;
}

/*----------------------------------------------------------------------
-- emit_decl_code - generate code for local object.
--
-- This routine generates code for identifier id to declare it in DSA
-- for the corresponding procedure. Generated code is emitted to final
-- output code. */

static void emit_decl_code(IDENT *id)
{     int seqn = id->block->seqn;
      switch (id->flags)
      {  case F_REAL:
            /* local real simple variable */
         case F_REAL | F_BYVAL:
            /* real simple formal parameter called by value */
            append(emit, "      double %s_%d;\n", id->name, seqn);
            break;
         case F_INT:
            /* local integer simple variable */
         case F_INT | F_BYVAL:
            /* integer simple formal parameter called by value */
            append(emit, "      int %s_%d;\n", id->name, seqn);
            break;
         case F_BOOL:
            /* local Boolean simple variable */
         case F_BOOL | F_BYVAL:
            /* Boolean simple formal parameter called by value */
            append(emit, "      bool %s_%d;\n", id->name, seqn);
            break;
         case F_LABEL | F_BYVAL:
            /* formal label called by value */
            append(emit, "      struct label %s_%d;\n", id->name, seqn);
            break;
         case F_REAL | F_ARRAY:
            /* local real array */
         case F_REAL | F_ARRAY | F_BYVAL:
            /* formal real array called by value */
         case F_REAL | F_ARRAY | F_BYNAME:
            /* formal real array called by name */
         case F_INT | F_ARRAY:
            /* local integer array */
         case F_INT | F_ARRAY | F_BYVAL:
            /* formal integer array called by value */
         case F_INT | F_ARRAY | F_BYNAME:
            /* formal integer array called by name */
         case F_BOOL | F_ARRAY:
            /* local Boolean array */
         case F_BOOL | F_ARRAY | F_BYVAL:
            /* formal Boolean array called by value */
         case F_BOOL | F_ARRAY | F_BYNAME:
            /* formal Boolean array called by name */
            append(emit, "      struct dv *%s_%d;\n", id->name, seqn);
            break;
         case F_REAL | F_BYNAME:
            /* real simple formal parameter called by name */
         case F_INT | F_BYNAME:
            /* integer simple formal parameter called by name */
         case F_BOOL | F_BYNAME:
            /* Boolean simple formal parameter called by name */
         case F_LABEL | F_BYNAME:
            /* formal label called by name */
         case F_SWITCH | F_BYNAME:
            /* formal switch */
         case F_REAL | F_PROC | F_BYNAME:
            /* formal real procedure */
         case F_INT | F_PROC | F_BYNAME:
            /* formal integer procedure */
         case F_BOOL | F_PROC | F_BYNAME:
            /* formal Boolean procedure */
         case F_PROC | F_BYNAME:
            /* formal typeless procedure */
            append(emit, "      struct arg %s_%d;\n", id->name, seqn);
            break;
         case F_STRING | F_BYNAME:
            /* formal string */
            append(emit, "      char *%s_%d;\n", id->name, seqn);
            break;
         case F_REAL | F_OWN:
            /* own real simple variable */
         case F_INT | F_OWN:
            /* own integer simple variable */
         case F_BOOL | F_OWN:
            /* own Boolean simple variable */
         case F_REAL | F_ARRAY | F_OWN:
            /* own real array */
         case F_INT | F_ARRAY | F_OWN:
            /* own integer array */
         case F_BOOL | F_ARRAY | F_OWN:
            /* own Boolean array */
         case F_LABEL:
            /* local label */
         case F_SWITCH:
            /* local switch */
         case F_REAL | F_PROC:
            /* local real procedure */
         case F_INT | F_PROC:
            /* local integer procedure */
         case F_BOOL | F_PROC:
            /* local Boolean procedure */
         case F_PROC:
            /* local typeless procedure */
            break;
         default:
            /* any other id->flags values are impossible */
            assert(id->flags != id->flags);
      }
      return;
}

/*----------------------------------------------------------------------
-- emit_dsa_code - generate code for all DSAs.
--
-- This routine generates code to declare DSA for all local procedures.
-- Generated code is emitted to final output code. */

static void emit_dsa_code(void)
{     BLOCK *block;
      for (block = first_b; block != NULL; block = block->next)
      {  IDENT *proc;
         BLOCK *b;
         proc = block->proc;
         if (proc == NULL || (proc->flags & (F_CODE | F_BLTIN)))
            continue; /* if it is not procedure block */
         /* standard procedure display */
         append(emit, "struct dsa_%s_%d\n",
            proc->name, proc->block->seqn);
         append(emit,
            "{     /* procedure %s (level %d) declared at line %d */\n",
            proc->name, dsa_level(proc) + 1, proc->ssn_decl);
         append(emit, "      char *proc;\n");
         append(emit, "      char *file;\n");
         append(emit, "      int line;\n");
         append(emit, "      struct dsa *parent;\n");
         append(emit, "      struct dsa *vector[%d+1];\n",
            dsa_level(proc) + 1);
         /* for each block level standard locations are needed:
            old_top  that keeps stack top value immediately after
                     entrance to a block;
            new_top  that keeps stack top value after allocation of
                     local arrays (or after copying formal arrays called
                     by value in the case of procedure block);
            jump     that keeps environment status; this is used by
                     go_to library routine performing global go to. */
         {  int maxlev = 0, k, need;
            for (b = first_b; b != NULL; b = b->next)
            {  if (proc_block(b) == block && maxlev < block_level(b))
                  maxlev = block_level(b);
            }
            append(emit, "      /* level of innermost block = %d */\n",
               maxlev);
            for (k = 0; k <= maxlev; k++)
            {  append(emit, "      struct mem *old_top_%d;\n", k);
               append(emit, "      struct mem *new_top_%d;\n", k);
               /* to decrease memory size used by DSA 'jump' of level k
                  is declared only if any block of level k has any label
                  which are actually used in designational expression
                  and to which global go to can lead */
               need = 0;
               for (b = first_b; b != NULL; b = b->next)
               {  if (proc_block(b) == block && block_level(b) == k &&
                      has_labels(b)) need = 1;
               }
               if (need) append(emit, "      jmp_buf jump_%d;\n", k);
            }
         }
         /* generate code for every object declared in each block of
            the corresponding procedure; all such objects are located
            in procedure DSA */
         for (b = first_b; b != NULL; b = b->next)
         {  IDENT *id;
            int count = 0;
            if (proc_block(b) != block) continue;
            /* generate code for individual block */
            append(emit, "      /* %s block %d (level %d) beginning at "
               "line %d */\n",
               b->proc != NULL ? "procedure" : "local",
               b->seqn, block_level(b), b->ssn);
            /* location for value returned by procedure */
            if (b->proc != NULL)
            {  assert(b->proc == proc);
               append(emit, "      struct desc retval;\n");
            }
            /* locations for local identifiers and formal parameters */
            for (id = b->first; id != NULL; id = id->next)
            {  append(emit, "      /* %s:", id->name);
               if (id->flags & F_OWN)     append(emit, " own");
               if (id->flags & F_BYVAL)   append(emit, " by value");
               if (id->flags & F_BYNAME)  append(emit, " by name");
               if (id->flags & F_REAL)    append(emit, " real");
               if (id->flags & F_INT)     append(emit, " integer");
               if (id->flags & F_BOOL)    append(emit, " Boolean");
               if (id->flags & F_LABEL)   append(emit, " label");
               if (id->flags & F_ARRAY)   append(emit, " array");
               if (id->flags & F_SWITCH)  append(emit, " switch");
               if (id->flags & F_PROC)    append(emit, " procedure");
               if (id->flags & F_STRING)  append(emit, " string");
               assert(!(id->flags & (F_CODE | F_BLTIN)));
               append(emit, "\n         %s at line %d and ",
                  id->flags & (F_BYVAL | F_BYNAME) ?
                     "specified" : "declared", id->ssn_decl);
               if (id->ssn_used == 0)
                  append(emit, "never referenced */\n");
               else
               {  append(emit, "first referenced at line %d */\n",
                     id->ssn_used);
                  if (id->flags == F_LABEL) id->dim = ++count;
               }
               emit_decl_code(id);
            }
         }
         append(emit, "};\n\n");
      }
      return;
}

/*----------------------------------------------------------------------
-- emit_startup_code - generate startup code.
--
-- This routine generates startup code to call main program (only if
-- translation unit contains main program). */

static void emit_startup_code(void)
{     append(emit, "int main(void)\n");
      append(emit, "{     /* Algol program startup code */\n");
      append(emit, "      main_program_0();\n");
      append(emit, "      return 0;\n");
      append(emit, "}\n\n");
      return;
}

/*----------------------------------------------------------------------
-- get_code_char - get next character of final output code.
--
-- This routine returns the next character of final output code as if
-- this code were a sequential text file. */

static CSQE *sqe_ptr; /* points to current output code entry */
static int sqe_pos;   /* position of current output code character */

static int get_code_char(void)
{     int c;
      for (;;)
      {  c = (unsigned char)sqe_ptr->str[sqe_pos++];
         if (c != '\0') break;
         sqe_ptr = sqe_ptr->next;
         if (sqe_ptr == NULL)
         {  c = 0x1A; /* end of code */
            break;
         }
         sqe_pos = 0;
      }
      return c;
}

/*----------------------------------------------------------------------
-- output_code - write output code to output text file.
--
-- This routine formats and writes final output code to output text
-- file. Formatting is used since some entries of code may be long. */

static void output_code(CODE *code)
{     int size;         /* length of current output line */
      int len;          /* length of current piece of code */
      char str[255+1];  /* the current piece of code */
      int c;
      assert(50 <= width && width <= 255);
      /* prepare to scan final output code */
      sqe_ptr = code->head;
      sqe_pos = 0;
      /* format output code and write it to output text file */
      size = 0;
      for (;;)
      {  /* accumulate the current piece of code */
         len = 0, str[0] = '\0';
         for (;;)
         {  c = get_code_char();
            if (c == 0x1A) break;
            assert(len < sizeof(str)-1);
            str[len++] = (char)c, str[len] = '\0';
            if (c == '\n' || c == ' ' || c == '(' || c == ')' ||
                c == ':' || c == ',' || c == ';' || c == '"') break;
         }
         /* if output lines becomes too long then insert break and
            change to a new line */
         if (size + len + (c == '\n' ? -1 : c == '"' ? 6 : 0) > width)
         {  if (size > 0)
            {  fputc('\n', outfile);
               fputs("         ", outfile), size = 9;
            }
         }
         /* write the next piece of output code */
         fputs(str, outfile);
         if (c == '\n') size = 0; else size += len;
         if (c == 0x1A) break;
         if (c == '"')
         {  /* literal output requires special processing */
            for (;;)
            {  int oldc = c;
               c = get_code_char();
               assert(c != 0x1A);
               if (size + 2 > width && oldc != '\\')
                  fputs("\"\n         \"", outfile), size = 10;
               fputc(c, outfile), size++;
               if (oldc != '\\' && c == '"') break;
            }
         }
      }
      return;
}

/*----------------------------------------------------------------------
-- display_help - display help.
--
-- This routine displays help information about the program as it is
-- required by the GNU Coding Standards. */

static void display_help(char *my_name)
{     printf("Usage: %s [options...] [filename]\n", my_name);
      printf("\n");
      printf("Options:\n");
      printf("   -d, --debug          run translator in debug mode\n");
      printf("   -e nnn, --errormax nnn\n");
      printf("                        maximal error allowance (0 <= nnn"
         " <= 255);\n");
      printf("                        default: -e 0 (continue translati"
         "on in any case)\n");
      printf("   -h, --help           display this help information and"
         " exit(0)\n");
      printf("   -l nnn, --linewidth nnn\n");
      printf("                        desirable output line width (50 <"
         "= nnn <= 255);\n");
      printf("                        default: -l 72\n");
      printf("   -o filename, --output filename\n");
      printf("                        send output C code to filename\n")
         ;
      printf("   -t, --notimestamp    suppress time stamp in output C c"
         "ode\n");
      printf("   -v, --version        display translator version and ex"
         "it(0)\n");
      printf("   -w, --nowarn         suppress all warning messages\n");
      printf("\n");
      printf("N.B.  The translator reads input file TWICE, therefore th"
         "is file should\n");
      printf("      be assigned to regular file (but not to terminal, p"
         "ipe, etc.)\n");
      printf("\n");
      printf("Please, report bugs to <bug-marst@gnu.org>\n");
      exit(EXIT_SUCCESS);
      /* no return */
}

/*----------------------------------------------------------------------
-- display_version - display version.
--
-- This routine displays version information for the program as it is
-- required by the GNU Coding Standards. */

static void display_version(void)
{     printf("                                 Was sich ueberhaupt sage"
         "n laesst, laesst\n");
      printf("                                 sich klar sagen; und wov"
         "on man nicht reden\n");
      printf("                                 kann, darueber muss man "
         "schweigen.\n");
      printf("                                                         "
         "Ludwig Wittgenstein\n");
      printf("\n");
      printf("%s\n", version);
      printf("Copyright (C) 2000, 2001, 2002, 2007, 2013 Free Software "
         "Foundation, Inc.\n");
      printf("This program is free software; you may redistribute it un"
         "der the terms of\n");
      printf("the GNU General Public License. This program has absolute"
         "ly no warranty.\n");
      exit(EXIT_SUCCESS);
      /* no return */
}

/*----------------------------------------------------------------------
-- process_cmdline - process command line parameters.
--
-- This routine processes parameters specified in command line. */

static void process_cmdline(int argc, char *argv[])
{     int k;
      for (k = 1; k < argc; k++)
      {  if (strcmp(argv[k], "-d") == 0 ||
             strcmp(argv[k], "--debug") == 0)
            debug = 1;
         else if (strcmp(argv[k], "-e") == 0 ||
                  strcmp(argv[k], "--errormax") == 0)
         {  char *endptr;
            k++;
            if (k == argc)
            {  fprintf(stderr, "No error count specified\n");
               exit(EXIT_FAILURE);
            }
            errmax = strtol(argv[k], &endptr, 10);
            if (!(*endptr == '\0' && 0 <= errmax && errmax <= 255))
            {  fprintf(stderr, "Invalid error count `%s'",
                  argv[k]);
               exit(EXIT_FAILURE);
            }
         }
         else if (strcmp(argv[k], "-h") == 0 ||
                  strcmp(argv[k], "--help") == 0)
            display_help(argv[0]);
         else if (strcmp(argv[k], "-l") == 0 ||
                  strcmp(argv[k], "--linewidth") == 0)
         {  char *endptr;
            k++;
            if (k == argc)
            {  fprintf(stderr, "No line width specified\n");
               exit(EXIT_FAILURE);
            }
            width = strtol(argv[k], &endptr, 10);
            if (!(*endptr == '\0' && 50 <= width && width <= 255))
            {  fprintf(stderr, "Invalid line width `%s'",
                  argv[k]);
               exit(EXIT_FAILURE);
            }
         }
         else if (strcmp(argv[k], "-o") == 0 ||
                  strcmp(argv[k], "--output") == 0)
         {  k++;
            if (k == argc)
            {  fprintf(stderr, "No output file name specified\n");
               exit(EXIT_FAILURE);
            }
            if (outfilename[0] != '\0')
            {  fprintf(stderr, "Only one output file allowed\n");
               exit(EXIT_FAILURE);
            }
            outfilename = argv[k];
         }
         else if (strcmp(argv[k], "-t") == 0 ||
                  strcmp(argv[k], "--notimestamp") == 0)
            time_stamp = 0;
         else if (strcmp(argv[k], "-v") == 0 ||
                  strcmp(argv[k], "--version") == 0)
            display_version();
         else if (strcmp(argv[k], "-w") == 0 ||
                  strcmp(argv[k], "--nowarn") == 0)
            warn = 0;
         else if (argv[k][0] == '-' ||
                 (argv[k][0] == '-' && argv[k][1] == '-'))
         {  fprintf(stderr, "Invalid option `%s'; try %s --help\n",
               argv[k], argv[0]);
            exit(EXIT_FAILURE);
         }
         else
         {  if (infilename[0] != '\0')
            {  fprintf(stderr, "Only one input file allowed\n");
               exit(EXIT_FAILURE);
            }
            infilename = argv[k];
         }
      }
      return;
}

/*----------------------------------------------------------------------
-- main - main routine.
--
-- This main routine is called by the control program and manages the
-- process of translation. */

int main(int argc, char *argv[])
{     /* process optional command line parameters */
      process_cmdline(argc, argv);
      /* open the input text file */
      if (infilename[0] == '\0')
      {  infilename = "(stdin)";
         infile = stdin;
      }
      else
      {  infile = fopen(infilename, "r");
         if (infile == NULL)
         {  fprintf(stderr, "Unable to open input file `%s' - %s\n",
               infilename, strerror(errno));
            exit(EXIT_FAILURE);
         }
      }
      /* open the output text file */
      if (outfilename[0] == '\0')
      {  outfilename = "(stdout)";
         outfile = stdout;
      }
      else
      {  outfile = fopen(outfilename, "w");
         if (outfile == NULL)
         {  fprintf(stderr, "Unable to open output file `%s' - %s\n",
               outfilename, strerror(errno));
            exit(EXIT_FAILURE);
         }
      }
      /*** start the first pass ***/
      first_pass = 1, second_pass = 0;
      first_b = last_b = current = NULL;
      if (debug)
         fprintf(outfile, "#if 0 /* start of translator debug output */"
            "\n\n");
      entire_module();
      if (debug)
         fprintf(outfile,
            "\n#endif /* end of translator debug output */\n\n");
      if (e_count)
      {  if (e_count == 1)
            error("one error detected on the first pass; translation te"
               "rminated");
         else
            error("%d errors detected on the first pass; translation te"
               "rminated", e_count);
         exit(EXIT_FAILURE);
      }
      /*** interlude ***/
      first_pass = 0, second_pass = 1;
      /* prepare to emit final output code */
      emit = new_code();
      append(emit, "/* %s */\n", outfilename);
      append(emit, "\n");
      append(emit, "/* generated by GNU %s */\n", version);
      if (time_stamp)
      {  time_t zeit = time(NULL);
         char stamp[25];
         memcpy(stamp, ctime(&zeit), 24);
         stamp[24] = '\0';
         append(emit, "/* %s */\n", stamp);
         append(emit, "/* source file: %s */\n", infilename);
         append(emit, "/* object file: %s */\n", outfilename);
      }
      append(emit, "\n");
      append(emit, "#include \"algol.h\"\n");
      append(emit, "\n");
      /* generate code for prototypes of local switches and translated
         procedures, because they may be used before their declarations
         appear */
      {  BLOCK *b;
         for (b = first_b; b != NULL; b = b->next)
         {  IDENT *id;
            if (b->proc != NULL)
               emit_proc_head(b->proc, 1);
            for (id = b->first; id != NULL; id = id->next)
            {  if (id->flags == F_SWITCH)
                  append(emit, "static struct label %s_%d /* local swit"
                     "ch */ (int);\n\n", id->name, id->block->seqn);
            }
         }
      }
      /* generate code to declare DSA structures for all procedures */
      emit_dsa_code();
      /*** start the second pass ***/
      last_b = NULL;
      rewind(infile);
      {  int is_main;
         is_main = entire_module();
         /* if main <program> appeared then startup code is needed */
         if (is_main) emit_startup_code();
      }
      if (e_count)
      {  if (e_count == 1)
            error("one error detected on the second pass; translation t"
               "erminated");
         else
            error("%d errors detected on the second pass; translation t"
               "erminated", e_count);
         exit(EXIT_FAILURE);
      }
      /* format and write output code to the output text file */
      append(emit, "/* eof */\n");
      output_code(emit);
      /* close the input text file */
      fclose(infile);
      /* close the output text file */
      fflush(outfile);
      if (ferror(outfile))
      {  fprintf(stderr, "Write error on `%s' - %s\n", outfilename,
            strerror(errno));
         exit(EXIT_FAILURE);
      }
      fclose(outfile);
      /* translation has been successfully completed */
      return 0;
}

/* eof */

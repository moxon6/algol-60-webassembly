/* macvt.c */

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

/*--------------------------------------------------------------------*/

static char *version = "MACVT -- Algol 60 Converter, Version 2.7";
/* Version information. */

static char *infilename = "";
/* The name of input text file that contains source Algol 60 program.
   If this file is the standard input file then its name is "(stdin)".
   This name used in error messages. */

static FILE *infile;
/* The stream connected to the input text file. */

static char *outfilename = "";
/* The name of output text file to which the converter emits converted
   Algol 60 program. If this file is the standard output file then its
   name is "(stdout)". */

static FILE *outfile;
/* The stream connected to the output text file. */

static int free_coding = 0;
/* Free coding flag. If this flag is set then free representation of
   basic symbols (excluding operators) is used. Otherwise if this flag
   is clear then classic representation of basic symbols is used. */

#define classic (!free_coding)

static int more_free = 0;
/* Free coding modifier. If this flag is set then free representation
   is used also for operators. This flag has sense only if free coding
   flag is set. */

static int old_sc = 0;
/* Old semicolon flag. If this flag is set then '.,' is recognized as
   semicolon (including comments) */

static int old_ten = 0;
/* Old ten symbol flag. If this flag is set then single apostrophe (if
   it is followed by +, -, or digit) is recognized as ten symbol. */

static int ignore_case = 0;
/* Ignore case flag. If this flag is set then all letters (excluding
   those in comments and strings) will be converted to lower case. */

/* Codes of basic symbols used by the converter: */
#define S_CHAR       -1 /* individual character (not symbol) */
#define S_EOF        0  /* _|_ */
#define S_LETTER     1  /* a, b, ..., z, A, B, ..., Z */
#define S_DIGIT      2  /* 0, 1, ..., 9 */
#define S_PLUS       3  /* + */
#define S_MINUS      4  /* - */
#define S_TIMES      5  /* * */
#define S_SLASH      6  /* / */
#define S_INTDIV     7  /* % */
#define S_POWER      8  /* **, ^ */
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
#define S_OPEN       30 /* openinig quote */
#define S_CLOSE      31 /* closing quote */
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

static int e_count = 0;
/* Error count. */

static int l_count = 0;
/* Source line count. This value is increased by one each time when the
   next line has been read from the input text file. */

static int ch;
/* Current (not processed yet) character. */

/*----------------------------------------------------------------------
-- my_assert - perform diagnostics.
--
-- This routine is used indirectly through 'assert' macro and checks
-- some error conditions that can happen only if there is a bug in the
-- converter. */

static void my_assert(char *expr, char *file, int line)
{     fprintf(stderr,
         "Internal converter error: %s, file %s, line %d\n",
         expr, file, line);
      fprintf(stderr, "Please, report to <bug-marst@gnu.org>\n");
      fflush(stderr);
      fflush(outfile);
      exit(EXIT_FAILURE);
      /* no return */
}

#undef assert

#define assert(expr)\
      ((void)((expr) || (my_assert(#expr, __FILE__, __LINE__), 1)))

/*----------------------------------------------------------------------
-- error - print error message.
--
-- This routine formats and sends to stderr a message that reflects a
-- coding error in source Algol 60 program. */

static void error(char *msg, ...)
{     va_list arg;
      fprintf(stderr, "%s:%d: ", infilename, l_count+1);
      va_start(arg, msg);
      vfprintf(stderr, msg, arg);
      va_end(arg);
      fprintf(stderr, "\n");
      e_count++;
      return;
}

/*----------------------------------------------------------------------
-- get_char - read next character.
--
-- This routine reads next character from the input text file assigning
-- its value to 'ch'. If end-of-file occured then special value 0x1A is
-- assigned to 'ch'. */

static void get_char(void)
{     ch = fgetc(infile);
      if (ferror(infile))
      {  fprintf(stderr, "Read error on `%s' - %s\n", infilename,
            strerror(errno));
         exit(EXIT_FAILURE);
      }
      if (feof(infile))
         ch = 0x1A;
      else if (iscntrl(ch) && !isspace(ch))
      {  error("invalid control character 0x%02X", ch);
         ch = ' ';
      }
      else if (ch == '\n')
         l_count++;
      return;
}

/*----------------------------------------------------------------------
-- scan_pad - scan non-significant characters.
--
-- This routine scans all characters that are non-significant (these
-- characters are immediately sent to output text file to keep original
-- formatting). After return the current character will be significant
-- one. */

static void emit_sym(int sym, int c);

static void scan_pad(int flag)
{     if (flag)
         while (isspace(ch)) emit_sym(S_CHAR, ch), get_char();
      return;
}

/*----------------------------------------------------------------------
-- scan_comment - scan comment sequence.
--
-- This routine scans all characters that follow 'comment' delimiter
-- until semicolon (these characters are sent to output text file). */

static void scan_comment(void)
{     if (isalnum(ch)) emit_sym(S_CHAR, ' ');
      for (;;)
      {  if (ch == 0x1A) break; /* terminated by eof */
         if (ch == ';') break;
         if (old_sc && ch == '.')
         {  /* semicolon can be coded as '.,' */
            get_char(), scan_pad(classic);
            if (ch == ',') break;
            emit_sym(S_CHAR, '.');
            if (ch == ';') break;
         }
         emit_sym(S_CHAR, ch), get_char();
      }
      emit_sym(S_SEMICOLON, 0), get_char();
      return;
}

/*----------------------------------------------------------------------
-- convert - perform conversion.
--
-- This routine scans input text file char by char, recognizes basic
-- symbols, and sends these symbols to formatting routine 'emit_sym'.
-- Standard technique based on finite automation modelling is used to
-- recognize basic symbols. */

static void convert(void)
{     /* get the first input character */
      get_char();
      /* main loop */
      for (;;)
      if (ch == 0x1A)
      {  /* end of input text file detected */
         emit_sym(S_EOF, 0);
         break;
      }
      else if (isspace(ch))
      {  /* all white-space (non-significant) characters are written to
            output text file to keep original formatting */
         emit_sym(S_CHAR, ch), get_char();
      }
      else if (isalpha(ch))
      {  if (classic)
         {  /* in classic representation all keywords should be quoted;
               hence a letter can't begin a keyword */
            if (ignore_case) ch = tolower(ch);
            emit_sym(S_LETTER, ch), get_char();
         }
         else
         {  /* in non-classic representation keyword can have form of
               letter sequence; we should be careful if something like
               'then123' is processed */
            char str[10+1];
            int len = 0;
            memset(str, '\0', 10+1);
again:      while (isalnum(ch))
            {  if (len == 10) goto ident;
               if (ignore_case) ch = tolower(ch);
               str[len++] = (char)ch, get_char();
            }
            if (more_free && strcmp(str, "div") == 0)
               emit_sym(S_INTDIV, 0);
            else if (more_free && strcmp(str, "power") == 0)
               emit_sym(S_POWER, 0);
            else if (more_free && strcmp(str, "pow") == 0)
               emit_sym(S_POWER, 0);
            else if (more_free && strcmp(str, "less") == 0)
               emit_sym(S_LESS, 0);
            else if (more_free && strcmp(str, "notgreater") == 0)
               emit_sym(S_NOTGREATER, 0);
            else if (more_free && strcmp(str, "equal") == 0)
               emit_sym(S_EQUAL, 0);
            else if (more_free && strcmp(str, "notless") == 0)
               emit_sym(S_NOTLESS, 0);
            else if (more_free && strcmp(str, "greater") == 0)
               emit_sym(S_GREATER, 0);
            else if (more_free && strcmp(str, "notequal") == 0)
               emit_sym(S_NOTEQUAL, 0);
            else if (more_free && strcmp(str, "equiv") == 0)
               emit_sym(S_EQUIV, 0);
            else if (more_free && strcmp(str, "impl") == 0)
               emit_sym(S_IMPL, 0);
            else if (more_free && strcmp(str, "or") == 0)
               emit_sym(S_OR, 0);
            else if (more_free && strcmp(str, "and") == 0)
               emit_sym(S_AND, 0);
            else if (more_free && strcmp(str, "not") == 0)
               emit_sym(S_NOT, 0);
            else if (strcmp(str, "array") == 0)
               emit_sym(S_ARRAY, 0);
            else if (strcmp(str, "begin") == 0)
               emit_sym(S_BEGIN, 0);
            else if (strcmp(str, "Boolean") == 0)
               emit_sym(S_BOOLEAN, 0);
            else if (strcmp(str, "boolean") == 0)
               emit_sym(S_BOOLEAN, 0);
            else if (strcmp(str, "code") == 0)
               emit_sym(S_CODE, 0);
            else if (strcmp(str, "comment") == 0)
            {  emit_sym(S_COMMENT, 0);
               scan_comment();
            }
            else if (strcmp(str, "do") == 0)
               emit_sym(S_DO, 0);
            else if (strcmp(str, "else") == 0)
               emit_sym(S_ELSE, 0);
            else if (strcmp(str, "end") == 0)
               emit_sym(S_END, 0);
            else if (strcmp(str, "false") == 0)
               emit_sym(S_FALSE, 0);
            else if (strcmp(str, "for") == 0)
               emit_sym(S_FOR, 0);
            else if (strcmp(str, "go") == 0 && ch == ' ')
            {  /* between 'go' and 'to' only blanks are allowed */
               while (ch == ' ')  emit_sym(S_CHAR, ' '), get_char();
               goto again;
            }
            else if (strcmp(str, "goto") == 0)
               emit_sym(S_GOTO, 0);
            else if (strcmp(str, "if") == 0)
               emit_sym(S_IF, 0);
            else if (strcmp(str, "integer") == 0)
               emit_sym(S_INTEGER, 0);
            else if (strcmp(str, "label") == 0)
               emit_sym(S_LABEL, 0);
            else if (strcmp(str, "own") == 0)
               emit_sym(S_OWN, 0);
            else if (strcmp(str, "procedure") == 0)
               emit_sym(S_PROCEDURE, 0);
            else if (strcmp(str, "real") == 0)
               emit_sym(S_REAL, 0);
            else if (strcmp(str, "step") == 0)
               emit_sym(S_STEP, 0);
            else if (strcmp(str, "string") == 0)
               emit_sym(S_STRING, 0);
            else if (strcmp(str, "switch") == 0)
               emit_sym(S_SWITCH, 0);
            else if (strcmp(str, "then") == 0)
               emit_sym(S_THEN, 0);
            else if (strcmp(str, "true") == 0)
               emit_sym(S_TRUE, 0);
            else if (strcmp(str, "until") == 0)
               emit_sym(S_UNTIL, 0);
            else if (strcmp(str, "value") == 0)
               emit_sym(S_VALUE, 0);
            else if (strcmp(str, "while") == 0)
               emit_sym(S_WHILE, 0);
            else
ident:      {  /* this is an identifier, not a keyword */
               char *t;
               for (t = str; *t; t++)
                  emit_sym(isalpha(*t) ? S_LETTER : S_DIGIT, *t);
               /* we should scan entire identifier to avoid processing
                  'abc123then' as 'abc123' 'then' */
               while (isalnum(ch))
               {  if (ignore_case) ch = tolower(ch);
                  emit_sym(isalpha(ch) ? S_LETTER : S_DIGIT, ch);
                  get_char();
               }
            }
         }
      }
      else if (isdigit(ch))
         emit_sym(S_DIGIT, ch), get_char();
      else if (ch == '+')
         emit_sym(S_PLUS, 0), get_char();
      else if (ch == '-')
      {  get_char(), scan_pad(classic);
         if (ch == '>')
            emit_sym(S_IMPL, 0), get_char();
         else
            emit_sym(S_MINUS, 0);
      }
      else if (ch == '*')
      {  get_char(), scan_pad(classic);
         if (ch == '*')
            emit_sym(S_POWER, 0), get_char();
         else
            emit_sym(S_TIMES, 0);
      }
      else if (ch == '/')
      {  get_char(), scan_pad(classic);
         if (ch == ')')
            emit_sym(S_ENDSUB, 0), get_char();
         else
            emit_sym(S_SLASH, 0);
      }
      else if (ch == '%')
         emit_sym(S_INTDIV, ch), get_char();
      else if (ch == '^')
         emit_sym(S_POWER, ch), get_char();
      else if (ch == '<')
      {  get_char(), scan_pad(classic);
         if (ch == '=')
            emit_sym(S_NOTGREATER, 0), get_char();
         else
            emit_sym(S_LESS, 0);
      }
      else if (ch == '=')
      {  get_char(), scan_pad(classic);
         if (ch == '=')
            emit_sym(S_EQUIV, 0), get_char();
         else
            emit_sym(S_EQUAL, 0);
      }
      else if (ch == '>')
      {  get_char(), scan_pad(classic);
         if (ch == '=')
            emit_sym(S_NOTLESS, 0), get_char();
         else
            emit_sym(S_GREATER, 0);
      }
      else if (ch == '!')
      {  get_char(), scan_pad(classic);
         if (ch == '=')
            emit_sym(S_NOTEQUAL, 0), get_char();
         else
            emit_sym(S_NOT, 0);
      }
      else if (ch == '|')
         emit_sym(S_OR, 0), get_char();
      else if (ch == '&')
         emit_sym(S_AND, 0), get_char();
      else if (ch == ',')
         emit_sym(S_COMMA, 0), get_char();
      else if (ch == '.')
      {  get_char(), scan_pad(classic);
         if (ch == '.')
         {  get_char(), scan_pad(classic);
            if (ch == '=')
               emit_sym(S_ASSIGN, 0), get_char();
            else
               emit_sym(S_COLON, 0);
         }
         else if (old_sc && ch == ',')
            emit_sym(S_SEMICOLON, 0), get_char();
         else if (ch == '=')
            emit_sym(S_ASSIGN, 0), get_char();
         else
            emit_sym(S_POINT, 0);
      }
      else if (ch == '#')
         emit_sym(S_TEN, 0), get_char();
      else if (ch == ':')
      {  get_char(), scan_pad(classic);
         if (ch == '=')
            emit_sym(S_ASSIGN, 0), get_char();
         else
            emit_sym(S_COLON, 0);
      }
      else if (ch == ';')
         emit_sym(S_SEMICOLON, 0), get_char();
      else if (ch == '(')
      {  get_char(), scan_pad(classic);
         if (ch == '/')
            emit_sym(S_BEGSUB, 0), get_char();
         else
            emit_sym(S_LEFT, 0);
      }
      else if (ch == ')')
         emit_sym(S_RIGHT, 0), get_char();
      else if (ch == '[')
         emit_sym(S_BEGSUB, 0), get_char();
      else if (ch == ']')
         emit_sym(S_ENDSUB, 0), get_char();
      else if (ch == '\'')
      {  /* quoted keyword; now all non-significant characters are
            skipped in any case */
         get_char(), scan_pad(1);
         /* if single quote is followed by +, -, or digit, it denotes
            ten symbol */
         if (old_ten && (ch == '+' || ch == '-' || isdigit(ch)))
            emit_sym(S_TEN, 0);
         else
         {  char str[10+1+1];
            int len = 0;
            memset(str, '\0', 10+1+1);
            while (isalnum(ch) || ispunct(ch))
            {  if (ch == '\'') break;
               if (len < 11) str[len++] = (char)tolower(ch);
               get_char(), scan_pad(1);
            }
            if (ch == '\'')
               get_char();
            else
               error("closing apostrophe missing");
            if (strcmp(str, "/") == 0)
               emit_sym(S_INTDIV, 0);
            else if (strcmp(str, "div") == 0)
               emit_sym(S_INTDIV, 0);
            else if (strcmp(str, "power") == 0)
               emit_sym(S_POWER, 0);
            else if (strcmp(str, "pow") == 0)
               emit_sym(S_POWER, 0);
            else if (strcmp(str, "less") == 0)
               emit_sym(S_LESS, 0);
            else if (strcmp(str, "notgreater") == 0)
               emit_sym(S_NOTGREATER, 0);
            else if (strcmp(str, "equal") == 0)
               emit_sym(S_EQUAL, 0);
            else if (strcmp(str, "notless") == 0)
               emit_sym(S_NOTLESS, 0);
            else if (strcmp(str, "greater") == 0)
               emit_sym(S_GREATER, 0);
            else if (strcmp(str, "notequal") == 0)
               emit_sym(S_NOTEQUAL, 0);
            else if (strcmp(str, "equiv") == 0)
               emit_sym(S_EQUIV, 0);
            else if (strcmp(str, "impl") == 0)
               emit_sym(S_IMPL, 0);
            else if (strcmp(str, "or") == 0)
               emit_sym(S_OR, 0);
            else if (strcmp(str, "and") == 0)
               emit_sym(S_AND, 0);
            else if (strcmp(str, "not") == 0)
               emit_sym(S_NOT, 0);
            else if (strcmp(str, "10") == 0)
               emit_sym(S_TEN, 0);
            else if (strcmp(str, "array") == 0)
               emit_sym(S_ARRAY, 0);
            else if (strcmp(str, "begin") == 0)
               emit_sym(S_BEGIN, 0);
            else if (strcmp(str, "boolean") == 0)
               emit_sym(S_BOOLEAN, 0);
            else if (strcmp(str, "code") == 0)
               emit_sym(S_CODE, 0);
            else if (strcmp(str, "comment") == 0)
            {  emit_sym(S_COMMENT, 0);
               scan_comment();
            }
            else if (strcmp(str, "do") == 0)
               emit_sym(S_DO, 0);
            else if (strcmp(str, "else") == 0)
               emit_sym(S_ELSE, 0);
            else if (strcmp(str, "end") == 0)
               emit_sym(S_END, 0);
            else if (strcmp(str, "false") == 0)
               emit_sym(S_FALSE, 0);
            else if (strcmp(str, "for") == 0)
               emit_sym(S_FOR, 0);
            else if (strcmp(str, "goto") == 0)
               emit_sym(S_GOTO, 0);
            else if (strcmp(str, "if") == 0)
               emit_sym(S_IF, 0);
            else if (strcmp(str, "integer") == 0)
               emit_sym(S_INTEGER, 0);
            else if (strcmp(str, "label") == 0)
               emit_sym(S_LABEL, 0);
            else if (strcmp(str, "own") == 0)
               emit_sym(S_OWN, 0);
            else if (strcmp(str, "procedure") == 0)
               emit_sym(S_PROCEDURE, 0);
            else if (strcmp(str, "real") == 0)
               emit_sym(S_REAL, 0);
            else if (strcmp(str, "step") == 0)
               emit_sym(S_STEP, 0);
            else if (strcmp(str, "string") == 0)
               emit_sym(S_STRING, 0);
            else if (strcmp(str, "switch") == 0)
               emit_sym(S_SWITCH, 0);
            else if (strcmp(str, "then") == 0)
               emit_sym(S_THEN, 0);
            else if (strcmp(str, "true") == 0)
               emit_sym(S_TRUE, 0);
            else if (strcmp(str, "until") == 0)
               emit_sym(S_UNTIL, 0);
            else if (strcmp(str, "value") == 0)
               emit_sym(S_VALUE, 0);
            else if (strcmp(str, "while") == 0)
               emit_sym(S_WHILE, 0);
            else
               error("keyword `%s%s' not recognized",
                  str, len <= 10 ? "" : "...");
         }
      }
      else if (ch == '\"' || ch == '`')
      {  /* actual string in form of "..." or `...' */
         int quote = ch == '`' ? '\'' : '\"';
         emit_sym(S_OPEN, 0), get_char();
         for (;;)
         {  if (ch == 0x1A || ch == quote)
               break;
            else if (iscntrl(ch))
            {  error("invalid control character 0x%02X in string", ch);
               emit_sym(S_CHAR, '?'), get_char();
            }
            else if (ch == '\\')
            {  emit_sym(S_CHAR, '\\'), get_char();
               if (ch == 0x1A) break;
               emit_sym(S_CHAR, ch), get_char();
            }
            else
               emit_sym(S_CHAR, ch), get_char();
         }
         emit_sym(S_CLOSE, 0), get_char();
      }
      else
      {  error("character `%c' (0x%02X) not recognized", ch, ch);
         emit_sym(S_CHAR, ' '), get_char();
      }
      return;
}

/*--------------------------------------------------------------------*/

static int status = 0;
/* current status of formatting routine:
   0 - next basic symbol (or non-significant char) is expected
   1 - comment sequence after 'comment' delimiter is processed
   2 - quoted string is processed */

static int last_char = 0;
/* last character output by formatting routine */

#define MAX_LEN 999
/* maximum length of character string in buffer (excluding '\0') */

static int cur_len = 0;
/* current length of character string in buffer (excluding '\0') */

static char buffer[MAX_LEN+1];
/* buffer to accumulate letters and digits as character string */

/*----------------------------------------------------------------------
-- emit_str - emit character string to output text file.
--
-- This routine emits character string str to output text file and
-- remembers last character of emitted string. */

void emit_str(char *str)
{     assert(strlen(str) > 0);
      fprintf(outfile, "%s", str);
      if (ferror(outfile))
      {  fprintf(stderr, "Write error on `%s' - %s\n", outfilename,
            strerror(errno));
         exit(EXIT_FAILURE);
      }
      last_char = (unsigned char)str[strlen(str)-1];
      return;
}

/*----------------------------------------------------------------------
-- emit_sym - emit basic symbol to output text file.
--
-- This routine emits basic symbol with given code to output text file
-- using hardware representation implemented in the MARST translator.
--
-- Argument sym is code of basic symbol, argument c is value of letter,
-- digit, or individual character. */

static void emit_sym(int sym, int c)
{     static char *word[] = /* keywords recognized by MARST */
      {  "array", "begin", "Boolean", "boolean", "code", "comment",
         "do", "else", "end", "false", "for", "go", "goto", "if",
         "integer", "label", "own", "procedure", "real", "step",
         "string", "switch", "then", "true", "until", "value", "while",
         NULL
      };
      if (status == 0)
      {  /* basic symbol (or non-significant char) is expected */
         /* if the current symbol is not a letter nor a digit then it's
            time to emit accumulated character string */
         if (!(sym == S_LETTER || sym == S_DIGIT) && cur_len > 0)
         {  /* we should be careful since character string can be the
               same as keyword */
            int k;
            for (k = 0; word[k] != NULL; k++)
            if (strcmp(buffer, word[k]) == 0)
            {  /* character string is the same as keyword; insert space
                  before last letter to prevent MARST to recognize such
                  sequence as a keyword */
               assert(cur_len < MAX_LEN);
               buffer[cur_len+1] = '\0';
               buffer[cur_len] = buffer[cur_len-1];
               buffer[cur_len-1] = ' ';
               break;
            }
            /* if last emitted character is a letter then insert space
               because it could be last character of a keyword */
            if (isalpha(last_char)) emit_str(" ");
            emit_str(buffer);
            /* now buffer is empty */
            cur_len = 0;
            buffer[0] = '\0';
         }
         /* emit given basic symbol */
         switch (sym)
         {  case S_CHAR:
               /* individual character must be non-significant */
               assert(isspace(c));
               {  char str[1+1];
                  str[0] = (char)c; str[1] = 0;
                  emit_str(str);
               }
               break;
            case S_EOF:
               /* this case is used only to flush accumulator */
               if (last_char != '\n') emit_str("\n"); break;
            case S_LETTER:
               /* letters are accumulated in buffer */
               assert(isalpha(c));
               assert(cur_len < MAX_LEN);
               buffer[cur_len++] = (char)c, buffer[cur_len] = '\0';
               break;
            case S_DIGIT:
               /* digits are accumulated in buffer too */
               assert(isdigit(c));
               assert(cur_len < MAX_LEN);
               buffer[cur_len++] = (char)c, buffer[cur_len] = '\0';
               break;
            case S_PLUS:
               emit_str("+"); break;
            case S_MINUS:
               emit_str("-"); break;
            case S_TIMES:
               if (last_char == '*') emit_str(" ");
               emit_str("*"); break;
            case S_SLASH:
               emit_str("/"); break;
            case S_INTDIV:
               emit_str("%"); break;
            case S_POWER:
               emit_str("^"); break;
            case S_LESS:
               emit_str("<"); break;
            case S_NOTGREATER:
               emit_str("<="); break;
            case S_EQUAL:
               if (last_char == '<' || last_char == '>' ||
                   last_char == '!' || last_char == ':') emit_str(" ");
               emit_str("="); break;
            case S_NOTLESS:
               emit_str(">="); break;
            case S_GREATER:
               if (last_char == '-') emit_str(" ");
               emit_str(">"); break;
            case S_NOTEQUAL:
               emit_str("!="); break;
            case S_EQUIV:
               emit_str("=="); break;
            case S_IMPL:
               emit_str("->"); break;
            case S_OR:
               emit_str("|"); break;
            case S_AND:
               emit_str("&"); break;
            case S_NOT:
               emit_str("!"); break;
            case S_COMMA:
               emit_str(","); break;
            case S_POINT:
               emit_str("."); break;
            case S_TEN:
               emit_str("#"); break;
            case S_COLON:
               emit_str(":"); break;
            case S_SEMICOLON:
               emit_str(";"); break;
            case S_ASSIGN:
               emit_str(":="); break;
            case S_LEFT:
               emit_str("("); break;
            case S_RIGHT:
               emit_str(")"); break;
            case S_BEGSUB:
               emit_str("["); break;
            case S_ENDSUB:
               emit_str("]"); break;
            case S_OPEN:
               if (last_char == '\"') emit_str(" ");
               emit_str("\""); status = 2; break;
            case S_CLOSE:
               assert(sym != sym);
            case S_ARRAY:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("array"); break;
            case S_BEGIN:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("begin"); break;
            case S_BOOLEAN:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("Boolean"); break;
            case S_CODE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("code"); break;
            case S_COMMENT:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("comment"); status = 1; break;
            case S_DO:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("do"); break;
            case S_ELSE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("else"); break;
            case S_END:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("end"); break;
            case S_FALSE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("false"); break;
            case S_FOR:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("for"); break;
            case S_GOTO:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("go to"); break;
            case S_IF:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("if"); break;
            case S_INTEGER:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("integer"); break;
            case S_LABEL:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("label"); break;
            case S_OWN:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("own"); break;
            case S_PROCEDURE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("procedure"); break;
            case S_REAL:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("real"); break;
            case S_STEP:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("step"); break;
            case S_STRING:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("string"); break;
            case S_SWITCH:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("switch"); break;
            case S_THEN:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("then"); break;
            case S_TRUE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("true"); break;
            case S_UNTIL:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("until"); break;
            case S_VALUE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("value"); break;
            case S_WHILE:
               if (isalnum(last_char)) emit_str(" ");
               emit_str("while"); break;
            default:
               assert(sym != sym);
         }
      }
      else if (status == 1)
      {  /* inside comment */
         switch (sym)
         {  case S_CHAR:
               assert(c != ';');
               {  char str[1+1];
                  str[0] = (char)c; str[1] = 0;
                  emit_str(str);
               }
               break;
            case S_SEMICOLON:
               emit_str(";"); status = 0; break;
            default:
               assert(sym != sym);
         }
      }
      else if (status == 2)
      {  /* inside quoted string */
         switch (sym)
         {  case S_CHAR:
               {  char str[1+1];
                  str[0] = (char)c; str[1] = 0;
                  emit_str(str);
               }
               break;
            case S_CLOSE:
               emit_str("\""); status = 0; break;
            default:
               assert(sym != sym);
         }
      }
      else
         assert(status != status);
      return;
}

/*----------------------------------------------------------------------
-- display_help - display help.
--
-- This routine displays help information about the program as required
-- by GNU Coding Standards. */

static void display_help(char *my_name)
{     printf("Usage: %s [options...] [filename]\n", my_name);
      printf("\n");
      printf("Options:\n");
      printf("   -c, --classic        classic representation used (defa"
         "ult)\n");
      printf("   -f, --free-coding    free representation used (excludi"
         "ng operators)\n");
      printf("   -h, --help           display this help information and"
         " exit(0)\n");
      printf("   -i, --ignore-case    convert to lower case\n");
      printf("   -m, --more-free      free representation used (includi"
         "ng operators)\n");
      printf("   -o filename, --output filename\n");
      printf("                        send converted Algol 60 program t"
         "o filename\n");
      printf("   -s, --old-sc         recognize ., as semicolon\n");
      printf("   -t, --old-ten        recognize single apostrophe as te"
         "n symbol\n");
      printf("   -v, --version        display converter version and exi"
         "t(0)\n");
      printf("\n");
      printf("Please, report bugs to <bug-marst@gnu.org>\n");
      exit(EXIT_SUCCESS);
      /* no return */
}

/*----------------------------------------------------------------------
-- display_version - display version.
--
-- This routine displays version information for the program as required
-- by GNU Coding Standards. */

static void display_version(void)
{     printf("%s\n", version);
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
-- process_cmdline - process command-line parameters.
--
-- This routine processes parameters specified in command line. */

static void process_cmdline(int argc, char *argv[])
{     int k;
      for (k = 1; k < argc; k++)
      {  if (strcmp(argv[k], "-c") == 0 ||
             strcmp(argv[k], "--classic") == 0)
            free_coding = 0, more_free = 0;
         else if (strcmp(argv[k], "-f") == 0 ||
                  strcmp(argv[k], "--free-coding") == 0)
            free_coding = 1, more_free = 0;
         else if (strcmp(argv[k], "-h") == 0 ||
                  strcmp(argv[k], "--help") == 0)
            display_help(argv[0]);
         else if (strcmp(argv[k], "-i") == 0 ||
                  strcmp(argv[k], "--ignore-case") == 0)
            ignore_case = 1;
         else if (strcmp(argv[k], "-m") == 0 ||
                  strcmp(argv[k], "--more-free") == 0)
            free_coding = 1, more_free = 1;
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
         else if (strcmp(argv[k], "-s") == 0 ||
                  strcmp(argv[k], "--old-sc") == 0)
            old_sc = 1;
         else if (strcmp(argv[k], "-t") == 0 ||
                  strcmp(argv[k], "--old-ten") == 0)
            old_ten = 1;
         else if (strcmp(argv[k], "-v") == 0 ||
                  strcmp(argv[k], "--version") == 0)
            display_version();
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
-- converting process. */

int main(int argc, char *argv[])
{     /* process optional command-line parameters */
      process_cmdline(argc, argv);
      /* open input text file */
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
      /* open output text file */
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
      /* perform conversion */
      convert();
      /* close input text file */
      fclose(infile);
      /* close output text file */
      fclose(outfile);
      /* exit to the control program */
      return e_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* eof */

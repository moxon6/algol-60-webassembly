comment /* alglib1.alg */

------------------------------------------------------------------------
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
-- but WITHOUT ANY WARRANTY., without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program. If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------;

comment STANDARD ALGOL 60 FUNCTIONS AND PROCEDURES;

comment Simple functions;

real procedure abs(E);
   value E; real E;
   abs := if E >= 0.0 then E else -E;

integer procedure iabs(E);
   value E; integer E;
   iabs := if E >= 0 then E else -E;

integer procedure sign(E);
   value E; real E;
   sign := if E > 0.0 then 1
      else if E < 0.0 then -1 else 0;

integer procedure entier(E);
   value E; real E;
   comment entier := largest integer not greater than E, i.e.
      E - 1 < entier <= E;
   begin
   integer j;
   j := E;
   entier := if j > E then j - 1 else j
   end entier;

comment Mathematical functions;

real procedure sqrt(E);
   value E; real E;
   if E < 0.0 then
      fault("negative sqrt", E)
   else
      inline("my_dsa.retval.u.real_val = sqrt(my_dsa.E_10);");

real procedure sin(E);
   value E; real E;
   comment sin := sine of E radians;
   inline("my_dsa.retval.u.real_val = sin(my_dsa.E_12);");

real procedure cos(E);
   value E; real E;
   comment cos := cosine of E radians;
   inline("my_dsa.retval.u.real_val = cos(my_dsa.E_14);");

real procedure arctan(E);
   value E; real E;
   comment arctan := principal value, in radians, of arctangent of E,
      i.e. -pi/2 <= arctan <= pi/2;
   inline("my_dsa.retval.u.real_val = atan(my_dsa.E_16);");

real procedure ln(E);
   value E; real E;
   comment ln := natural logarithm of E;
   if E <= 0.0 then
      fault("ln not positive", E)
   else
      inline("my_dsa.retval.u.real_val = log(my_dsa.E_18);");

real procedure exp(E);
   value E; real E;
   comment exp := exponential function of E;
   if E > ln(maxreal) then
      fault("overflow on exp", E)
   else
      inline("my_dsa.retval.u.real_val = exp(my_dsa.E_20);");

comment Terminating procedures;

procedure stop;
   inline("exit(EXIT_SUCCESS);");

procedure fault(str, r);
   value r; string str; real r;
   inline("fault(\"%s \" REAL_FMT, my_dsa.str_24, my_dsa.r_24);");

comment Input/output procedures;

procedure inchar(channel, str, int);
   value channel;
   integer channel, int; string str;
   comment Set int to value corresponding to the first position in str
   of current character on channel. Set int to zero if character not in
   str. Move channel pointer to next character;
   begin
   integer val;
   inline("{  char *ptr;");
   inline("   ptr = strchr(my_dsa.str_26, inchar(my_dsa.channel_26));");
   inline("   my_dsa.val_28 = (ptr == NULL ? 0 : (ptr - my_dsa.str_26) "
      "+ 1);");
   inline("}");
   int := val
   end inchar;

procedure outchar(channel, str, int);
   value channel, int;
   integer channel, int; string str;
   comment Pass to channel the character in str, corresponding to the
   value of int;
   if int < 1 | int > length(str) then
      fault("character not in string", int)
   else
      inline("outchar(my_dsa.channel_29, my_dsa.str_29[my_dsa.int_29 - "
         "1]);");

integer procedure length(str);
   comment length := number of characters in the string;
   string str;
   inline("my_dsa.retval.u.int_val = strlen(my_dsa.str_31);");

procedure outstring(channel, str);
   value channel;
   integer channel; string str;
   inline("outstring(my_dsa.channel_33, my_dsa.str_33);");

procedure outterminator(channel);
   value channel; integer channel;
   comment Outputs a terminator for use after a number;
   inline("outchar(my_dsa.channel_35, 0x20);");

procedure ininteger(channel, int);
   value channel; integer channel, int;
   comment int takes the value of an integer read from channel;
   begin
   integer val;
   inline("my_dsa.val_39 = ininteger(my_dsa.channel_37);");
   int := val
   end ininteger;

procedure outinteger(channel, int);
   value channel, int;
   integer channel, int;
   comment Passes to channel the characters representing the value of
   int, followed by a terminator;
   inline("outinteger(my_dsa.channel_40, my_dsa.int_40);");

procedure inreal(channel, re);
   value channel;
   integer channel; real re;
   comment re takes the value of a number read from channel;
   begin
   real val;
   inline("my_dsa.val_44 = inreal(my_dsa.channel_42);");
   re := val
   end inreal;

procedure outreal(channel, re);
   value channel, re;
   integer channel; real re;
   comment Passes to channel the characters representing the value of
   re, followed by a terminator;
   inline("outreal(my_dsa.channel_45, my_dsa.re_45);");

comment Environmental enquiries;

real procedure maxreal;
   inline("my_dsa.retval.u.real_val = DBL_MAX;");

real procedure minreal;
   inline("my_dsa.retval.u.real_val = DBL_MIN;");

integer procedure maxint;
   inline("my_dsa.retval.u.int_val = INT_MAX;");

comment maxreal, minreal, and maxint are, respectively, the maximum
allowable positive real number, the minimum allowable positive real
number, and the maximum allowable positive integer, such that any valid
expression of the form <primary> <arithmetic operator> <primary> will
be correctly evaluated, provided that each of the primaries concerned,
and the mathematically correct result lies within the open interval
(-maxreal, -minreal) or (minreal, maxreal) or is zero if of real type,
or within the open interval (-maxint, maxint) if of integer type. If
the result is of real type, the words 'correctly evaluated' must be
understood in the sense of numerical analysis;

real procedure epsilon;
   comment The smallest positive real number such that the computa-
   tional result of 1.0 + epsilon is greater than 1.0 and the computa-
   tional result of 1.0 - epsilon is less than 1.0;
   inline("my_dsa.retval.u.real_val = DBL_EPSILON;");

comment /* eof */;

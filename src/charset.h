/* charset.h - interface for retrieving environment charset.
   Copyright (C) 1992, 1998, 2001, 2007, 2009-2011 Free Software Foundation,
   Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston, MA
   02110-1301, USA.  */
   
#ifndef CHARSET_H
#define CHARSET_H 1

extern const char* OS_ENV;

char* get_charset();
char* get_input_charset();
char* get_output_charset();

int utf8len(char *s);
char* strtoutf8(char* cs_from, char* str);
char* utf8tostr(char* cs_to, char* str);

int output(FILE* stream, char* str);

#endif
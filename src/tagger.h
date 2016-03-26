/* tagger.c - main driver file for tagger.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <http://www.gnu.org/licenses/>
    or write to the Free Software  Foundation, Inc., 51 Franklin Street
    - Fifth Floor, Boston, MA 02110-1301, USA.  */
   
#ifndef TAGGER_H
#define TAGGER_H 1


/* type to handle operation (as argument from the command line)
 structure consists of a string and a pointer to the function to handle the operation
*/
struct operation {
    char* name;
    void (*f)(int, char*[], int);
};

/* Set up tagger database. */
void op_init(int argc, char* argv[], int index);

/* Create a new empty tag. */
void op_create(int argc, char* argv[], int index);

/* Create a new tag that contains all relations of a given tag. */
void op_clone(int argc, char* argv[], int index);

/* Destroy one or more element(s). */
void op_delete(int argc, char* argv[], int index);

/* Recover previously deleted element(s). */
void op_recover(int argc, char* argv[], int index);
    
/* Change the name of specified tag to given name. */
void op_rename(int argc, char* argv[], int index);

/* Merge two tags. */
void op_merge(int argc, char* argv[], int index);

/* Add one or more tag(s) to a file. */
void op_tag(int argc, char* argv[], int index);

/* Retrieve all files matching given criteria. */
void op_files(int argc, char* argv[], int index);

/* Show all tags applied to specified file(s). */
void op_tags(int argc, char* argv[], int index);

/* Show all elements from specified type (set in mode_flag). */
void op_list(int argc, char* argv[], int index);

/* Retrieve all elements matching given criteria. */
void op_query(int argc, char* argv[], int index);

#endif
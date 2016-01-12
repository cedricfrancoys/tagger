/* elem.h - interface for managing elements relations.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/
   
#ifndef ELEM_H
#define ELEM_H 1

#include "list.h"

#define ELEM_TAG    1
#define ELEM_FILE   2

#define ELEM_ADD   '+'
#define ELEM_REM   '-'

#define ELEM_NAME_MAX 1024


typedef struct elem {
    int type;
    char* name;
    char* file;
} ELEM;

char* resolve_name(int type, char* elem_name);
int update_record(char status, char* file, char* name);

int elem_init(int type, char* name, ELEM* el, int flag_create);
int elem_relate(char action, ELEM* elem1, ELEM* elem2);

/* Populate a list with nodes holding names of the elements pointed by the given element. */
int elem_retrieve_list(ELEM* elem, LIST* list);

/* Populate a list with nodes holding names of all elements of given type. */
int type_retrieve_list(int type, LIST* list);

/* Populate a list with nodes matching the given wildcard. */
int glob_retrieve_list(int type, char *wildcard, LIST* list);

/* Populate a destination list with nodes holding values of elements related to those in given list. */
int list_retrieve_list(int type, LIST* elems, LIST* list);
#endif
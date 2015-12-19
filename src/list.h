/* list.c - interface for manipulating sorted arrays of strings.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

#ifndef LIST_H
#define LIST_H 1
/* This file defines a linked list structure allowing to hold non-redundant and ordered array of strings
*/

/* Linked list node */
struct node {
    char* str;
    struct node *next;
};
typedef struct node NODE;

/* Linked list */
typedef struct list {
    NODE* first;
    int count;
} LIST;

/* Populate a list with nodes matchning the given wildcard.
*/
int glob_retrieve_list(char *wildcard, LIST* list);

/* Insert elem into a sorted list.
*/
int list_insert_unique(LIST* list, NODE* node);

/* Print out a list of names.
*/
int list_output(LIST* list);

/* Deallocate memory used for nodes contained in the list.
*/
void list_free(LIST* list);

#endif
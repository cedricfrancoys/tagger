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


int list_pos(LIST* list, NODE* node, NODE** nodeptr);

/* Insert elem into a sorted list. */
int list_insert_unique(LIST* list, NODE* node);

/* Remove all entries from list1 that are not also present in list2. */
int list_intersect(LIST* list1, LIST* list2);

/* Remove all entries from list1 that are present in list2. */
int list_diff(LIST* list1, LIST* list2);

/* Add each entry in list2 to list1. */
int list_merge(LIST* list1, LIST* list2);

/* Print out a list of names. */
int list_output(LIST* list);

/* Deallocate memory used for nodes contained in the list. */
void list_free(LIST* list);

#endif
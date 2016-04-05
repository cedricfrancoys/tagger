/* list.c - interface for manipulating sorted arrays of strings.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>


#include "env.h"
#include "charset.h"
#include "xalloc.h"
#include "list.h"


/*
 Returns a pointer to the node after which given node has to be inserted.
 If an error occurs or if an equivalent node is already present, function returns NULL.
*/
int list_pos(LIST* list, NODE* node, NODE** nodeptr) {
    if(!list || !node) return -1;
	int res = 1;
    NODE* ptr = list->first;
    while(ptr->next) {
        int cmp = strcmp(ptr->next->str, node->str);
        if(cmp == 0) {
            // an elem by that name is already in the list
			res = 0;
            break;
        }
        if(cmp > 0) {
            // we reached to position where the node has to be inserted
			res = 1;
            break;
        }
        ptr = ptr->next;
    }
	if(nodeptr != NULL) {
		*nodeptr = ptr;
	}
	return res;
}

/* Insert a node into a sorted list.
 (given node is ignored if a node holding an identical string is already in the list)
 return values:
 -1 error (NULL parameter(s))
  0 elem by that name already exists
  1 node successfuly inserted
*/
int list_insert_unique(LIST* list, NODE* node) {
    if(!list || !node) return -1;
	// find first occurence of node string in given list
    NODE* ptr;
	if(!list_pos(list, node, &ptr)) {
		// string already in the list
		return 0;
	}
    // insert node at current position
    NODE* temp = ptr->next;
    ptr->next = node;
    node->next = temp;
    ++list->count;
    return 1;
}

/* Remove all entries from list1 that are not also present in list2. 
*/
int list_intersect(LIST* list1, LIST* list2) {
    if(!list1 || !list2) return -1;

	NODE* ptr = list1->first;
	while(ptr->next) {
		if(list_pos(list2, ptr->next, NULL)) {
			// if string is not present in list2, remove it from list1
			NODE* temp = ptr->next;
			ptr->next = temp->next;
			free(temp->str);
			free(temp);			
			--list1->count;
		}
        else ptr = ptr->next;
		if(!ptr) break;
	}
    return 1;
}

/* Remove all entries from list1 that are present in list2. 
*/
int list_diff(LIST* list1, LIST* list2) {
    if(!list1 || !list2) return -1;

	NODE* ptr = list2->first;
	while(ptr->next) {
		NODE* ptr1;
		if(!list_pos(list1, ptr->next, &ptr1)) {
			// if string is present in both lists, remove it from list1
			NODE* temp = ptr1->next;
			ptr1->next = temp->next;
			free(temp->str);
			free(temp);
			--list1->count;
		}
        else ptr = ptr->next;
		if(!ptr) break;
	}	
    return 1;
}

/* Add each entry in list2 to list1.
*/
int list_merge(LIST* list1, LIST* list2) {
    if(!list1 || !list2) return -1;

	NODE* ptr = list2->first;
	while(ptr->next) {
		NODE* new = (NODE*) xzalloc(sizeof(NODE));
		new->str = (char*) xmalloc(strlen(ptr->next->str)+1);
		strcpy(new->str, ptr->next->str);
		list_insert_unique(list1, new);
		ptr = ptr->next;
	}	
    return 1;
}

/* Print out the content of a list.
 (i.e.: content of the str member of each node, separated by a new line char)
*/
int list_output(LIST* list){
	if(!list) return 0;
    for(NODE* node = list->first->next; node; node = node->next) {
        if(!output(stdout, node->str)) {
            return 0;
        }
        printf("\n");
    }
    return 1;
}

/* Deallocate memory used for nodes contained in the list.
*/
void list_free(LIST* list) {
    NODE* node = list->first;
    while(node) {
        NODE* next = node->next;
        free(node->str);
        free(node);
        node = next;
    }
    list->count = 0;
}

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
#include <errno.h>
#include <glob.h>

#include "env.h"
#include "charset.h"
#include "xalloc.h"
#include "list.h"

/* Populate a list with nodes holding absolute filenames matchning the given wildcard.
 (this function calls list_insert_unique, which avoid duplicates)
*/
int glob_retrieve_list(char *wildcard, LIST* list) {
    glob_t result;
    int g_res;
    if((g_res = glob(wildcard, GLOB_NOSORT|GLOB_NOCHECK, 0, &result)) != 0) {
        errno = g_res;
        return 0;
    }
    char* absolute_name;
    for(int i = result.gl_offs; result.gl_pathv[i]; ++i) {
        // retrieve absolute name of each file
        absolute_name = absolute_path(result.gl_pathv[i]);
        NODE* node = xmalloc(sizeof(node));
        node->str = xmalloc(strlen(absolute_name)+1);
        strcpy(node->str, absolute_name);
        // insert filenames into a list (best effort: no error check here)
        list_insert_unique(list, node);
    }
    globfree(&result);
    return 1;
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
    NODE* ptr = list->first;
    while(ptr->next) {
        int cmp = strcmp(ptr->next->str, node->str);
        if(cmp == 0) {
            // an elem by that name is already in the list
            return 0;
        }
        if(cmp > 0) {
            // we reached to position where the node has to be inserted
            break;
        }
        ptr = ptr->next;
    }
    // insert node at current position
    NODE* temp = ptr->next;
    ptr->next = node;
    node->next = temp;
    ++list->count;
    return 1;
}

/* Print out the content of a list.
 (i.e.: content of the str member of each node, separated by a new line char)
*/
int list_output(LIST* list){
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
    // We don't know how was allocated the root node
    NODE* node = list->first->next;
    while(node) {
        NODE* next = node->next;
        free(node->str);
        free(node);
        node = next;
    }
    list->count = 0;
}
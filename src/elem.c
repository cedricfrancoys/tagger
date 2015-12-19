/* elem.c - interface for managing elements relations.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

/* An element is either a tag or a file 
 (i.e. : 'node' is more appropriate since this applies to directories as well)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xalloc.h"
#include "env.h"
#include "hash.h"
#include "elem.h"

/* ELEM_DIR is defined in env.c
 Array holding the names of the sub-directories for database.
*/
extern const char* ELEM_DIR[];


/* Find the hashed filename (with full path) associated to an element (tag or file).
 In case of collision, name is resolved by adding an extension with an increment.
 (no file is created by this funciton)
*/
char* resolve_name(int type, char* elem_name) {
// if( !elem_name || (type != ELEM_FILE && type != ELEM_TAG)) return NULL;
    char* install_dir = get_install_dir();
    char* elem_id = hash(elem_name);
    // we add an extra 3 chars for optional increment (in case of collision), and 2 chars for slashes
    char* elem_file = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[type])+strlen(elem_id)+3+2+1);
    sprintf(elem_file, "%s/%s/%s", install_dir, ELEM_DIR[type], elem_id);

    // check if the file already exists
    FILE* stream = fopen(elem_file, "r,ccs=UTF-8");
    if(stream != NULL) {
        // a file by that name already exists
        char temp_name[ELEM_NAME_MAX];
        // temporary filename
        char* temp_file = xmalloc(strlen(elem_file)+1);
        strcpy(temp_file, elem_file);
        // while a file by that name already exists, check the content and increment the name
        for(int inc = 1; stream != NULL; ++inc) {
            // look into the file to check its related name
            if (fgets(temp_name, ELEM_NAME_MAX, stream) == NULL) {
                // unable to read from file : skip it
            }
            else {            
                // remove the newline char
                temp_name[strlen(temp_name)-1] = 0;
                if(strcmp(temp_name, elem_name) == 0) {
                    // we found the file associated with the given name
                    fclose(stream);
                    break;
                }
            }
            sprintf(temp_file, "%s.%02d", elem_file, inc);
            fclose(stream);
            stream = fopen(temp_file, "r,ccs=UTF-8");
        }
        strcpy(elem_file, temp_file);
        free(temp_file);
    }
    return elem_file;
}

/* Look into specified file for a line matching given name, and if it already exists set it to new status.
*/
int update_record(char status, char* file, char* name) {
    int result = 0;
    long int pos;
    char line[ELEM_NAME_MAX];
    FILE* fp = fopen(file, "r+,ccs=UTF-8");

    if(fp == NULL) {
        // something went wrong
        return -1;
    }    
    // skip the first line (full name of the element)
    fgets(line, ELEM_NAME_MAX, fp);
    pos = ftell(fp);
    while(fgets(line, ELEM_NAME_MAX, fp)) {
        // remove the new line char
        line[strlen(line)-1] = 0;
        if(strcmp(line+1, name) == 0) {
            // we found the element, go back to the beginning of the line
            fseek(fp, pos, SEEK_SET);
            fprintf(fp, "%c%s", status, name);
            result = 1;
            break;
        }
        pos = ftell(fp);
    }
    fclose(fp);
    return result;
}

/*
 return values:
 -1 error occured
  0 element does not exist and was not created
  1 element already exists (and was not created)
  2 element was created  
*/
int elem_init(int type, char* name, ELEM* el, int flag_create) {
    ELEM temp;
    if(el == NULL) {
        el = &temp;
    }
    el->type = type;
    el->name = xmalloc(strlen(name)+1);
    strcpy(el->name, name);
    el->file = resolve_name(el->type, el->name);
    
    FILE* fp = fopen(el->file, "r,ccs=UTF-8");
    if(fp != NULL) {
        // file already exists and we assume it is consistent (i.e.: full name as first line)
        fclose(fp);
        return 1;
    }
    else if(flag_create) {
        // file does not exist yet
        fp = fopen(el->file, "w,ccs=UTF-8");
        if(fp == NULL) {
            // error at file creation
            return -1;
        }
        // add a first line containing the full name of the element
        fprintf(fp, "%s\n", el->name);
        fclose(fp);
        return 2;
    }
    return 0;
}


/* Create or suppress a symetrical relation between given elements
 return codes:
 -1 : error
 0  : nothing done
 1  : updated relation
 2  : new relation
 
 Relations are defined using lines starting with a '+' (or '-' if relation was removed) and ending with a '\n'.
 Under windows, if user edit file manually, end of line might be changed into "\r\n" 
 which would prevent name detection (this possibility is not handled here).
*/
int elem_relate(char action, ELEM* elem1, ELEM* elem2) {
    int result = 0;
    // check that we were given actual elements
    if(elem1 == NULL || elem2 == NULL) {
        return -1;
    }
    if(elem1->type == elem2->type) {
        // cannot relate two elements of the same type (i.e. tagging a tag)
        return -1;
    }

    // we assume consistency : there can be only one or zero line matching the related elem
    
    // try to update the relation if it already exists (if so, we overwrite it)
    int res;
    res = update_record(action, elem1->file, elem2->name);
    if(res == 1) {
        result = 1;
    }
    if(res == 0 && action == ELEM_ADD) {
        // relation was not found and we need to create the relation
        FILE* fp = fopen(elem1->file, "a,ccs=UTF-8");
        // append a new line at the end of the file
        fprintf(fp, "%c%s\n", ELEM_ADD, elem2->name);
        fclose(fp);
        result = 2;
    }
    // do the same for symetrical relation
    res = update_record(action, elem2->file, elem1->name);
    if(res == 0 && action == ELEM_ADD) {
        FILE* fp = fopen(elem2->file, "a,ccs=UTF-8");
        fprintf(fp, "%c%s\n", ELEM_ADD, elem1->name);
        fclose(fp);
    }
    return result;
}


/* Populate a list with nodes holding names of the elements pointed by the given element.
 (this function calls list_insert_unique, which avoid duplicates)
*/
int elem_retrieve_list(ELEM* elem, LIST* list) {
    int result = 0;
    char line[ELEM_NAME_MAX];
    FILE* fp = fopen(elem->file, "r,ccs=UTF-8");
    if(fp == NULL) {
        return -1;
    }
    // skip the first line (full name of the element)
    fgets(line, ELEM_NAME_MAX, fp);
    while(fgets(line, ELEM_NAME_MAX, fp)) {
        // ignore obsolete relations
        if(line[0] == '+') {
            // remove the last char ('\n')
            line[strlen(line)-1] = 0;
            // add record to result list
            NODE* node = xmalloc(sizeof(node));
            node->str = xmalloc(strlen(line)+1);
            //copy the line, omitting first char ('+' or '-')
            strcpy(node->str, line+1);            
            // though we should'nt encounter any duplicate, if we do anyway, just ignore them
            if( list_insert_unique(list, node) < 0 ) {
                // unexpected error occured 
                return -1;
            }
        }
    }
    fclose(fp);
    return result;
}
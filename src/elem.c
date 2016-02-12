/* elem.c - interface for managing elements relations.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

/* An element is either a tag or a file 
 (i.e. : 'node' is more appropriate since this applies to directories as well)
 an element can be retrieved with its hash code (32-char md5 digest)
 collisions are resolved with additional increment (.%02d)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <glob.h>
#include <fnmatch.h>
#include <errno.h>

#include "xalloc.h"
#include "env.h"
#include "hash.h"
#include "list.h"
#include "elem.h"

/* ELEM_DIR is defined in env.c
 Array holding the names of the sub-directories for database.
*/
extern const char* ELEM_DIR[];

/* Checks if a file matches a given element name
*/
int check_file(char* elem_name, char* file_name){
    int res = -1;
    char temp_name[ELEM_NAME_MAX];
    FILE* stream = fopen(file_name, "r");
    if(stream == NULL) {
        // file does not exist yet
        res = 0;
    }
    else {
        // read first line
        if (fgets(temp_name, ELEM_NAME_MAX, stream)) {
            // remove newline char
            temp_name[strlen(temp_name)-1] = 0;
            if(strcmp(temp_name, elem_name) == 0) {
                // match
                res = 1;
            }
        }
        fclose(stream);
    }
    return res;
}

/* Find the hashed filename (with full path) associated to an element (tag or file).
 In case of collision, name is resolved by adding an extension with an increment.
 (this function do not create new file and always returns a filename)
*/
char* resolve_name(int type, char* elem_name) {
// if( !elem_name || (type != ELEM_FILE && type != ELEM_TAG)) return NULL;
    char* install_dir = get_install_dir();
    char* elem_id = hash(elem_name);
    // we add an extra 3 chars for optional increment (in case of collision), and 2 chars for slashes
    char* elem_file = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[type])+strlen(elem_id)+3+2+1);
    sprintf(elem_file, "%s/%s/%s", install_dir, ELEM_DIR[type], elem_id);

    // while a file by that name already exists (and is not related to the same element)
    for(int inc = 1; check_file(elem_name, elem_file) < 0; ++inc) {
        // increment the name
        sprintf(elem_file, "%s.%02d", elem_file, inc);
    }
    
    return elem_file;
}

/* Look into specified file for a line matching given name, and if it already exists set it to new status.
*/
int update_record(char status, char* file, char* name) {
    int result = 0;
    long int pos;
    char line[ELEM_NAME_MAX];
    FILE* fp = fopen(file, "r+");

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
    
    FILE* fp = fopen(el->file, "r");
    if(fp != NULL) {
        // file already exists and we assume it is consistent (i.e.: full name as first line)
        fclose(fp);
        return 1;
    }
    else if(flag_create) {
        // file does not exist yet
        fp = fopen(el->file, "w");
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


/* Create or suppress a symetrical relation between given elements.
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

    // we assume consistency, i.e. there is only one or zero line matching the related elem
    
    // try to update the relation if it already exists (if so, we overwrite it)
    int res;
    res = update_record(action, elem1->file, elem2->name);
    if(res == 1) {
        result = 1;
    }
    if(res == 0 && action == ELEM_ADD) {
        // relation was not found and we need to create the relation
        FILE* fp = fopen(elem1->file, "a");
        // append a new line at the end of the file
        fprintf(fp, "%c%s\n", ELEM_ADD, elem2->name);
        fclose(fp);
        result = 2;
    }
    // do the same for symetrical relation
    res = update_record(action, elem2->file, elem1->name);
    if(res == 0 && action == ELEM_ADD) {
        FILE* fp = fopen(elem2->file, "a");
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
    FILE* fp = fopen(elem->file, "r");
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
            NODE* node = xzalloc(sizeof(NODE));
            node->str = (char*) xmalloc(strlen(line)+1);
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

/* Populate a list with nodes holding names of all elements of given type.
*/
int type_retrieve_list(int type, LIST* list) {
    // obtain type-specific directory
    char* install_dir = get_install_dir();
    // allocate path, adding an extra char for slash/separator
    char* elems_dir = (char*) xmalloc(strlen(install_dir)+strlen(ELEM_DIR[type])+2);
    sprintf(elems_dir, "%s/%s", install_dir, ELEM_DIR[type]);

	struct dirent *ep;
	char elem_name[ELEM_NAME_MAX];
	DIR *dp = opendir(elems_dir);	
	if (!dp) return 0;
	else {
		while (ep = readdir(dp)) {
			if(strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0 && strstr(ep->d_name, ".trash") == NULL) {                
				char* elem_file = xmalloc(strlen(elems_dir)+strlen(ep->d_name)+2);
				sprintf(elem_file, "%s/%s", elems_dir, ep->d_name);
				FILE* stream;
                if(stream = fopen(elem_file, "r")) {
                    // read first line
                    if (fgets (elem_name, ELEM_NAME_MAX, stream) == NULL) {
                        // unable to read from file
                        fclose(stream);
                    }
                    else {
                        // remove the newline char
                        elem_name[strlen(elem_name)-1] = 0;
                        NODE* node = xmalloc(sizeof(NODE));
                        node->str = xmalloc(strlen(elem_name)+1);
                        strcpy(node->str, elem_name);
                        list_insert_unique(list, node);
                    }
                    fclose(stream);
                }
				free(elem_file);
			}
		}
		closedir (dp);
	}
	free(elems_dir);
    return 1;
}

/* Populate a list with nodes holding strings matching the given wildcard
 List content depends on given type:
 ELEM_FILE: absolute filenames matching wildcard
 ELEM_TAG:  tag names matching wildcard
 (this function calls list_insert_unique, which avoid duplicates)
*/
int glob_retrieve_list(int glob_type, int elem_type, char *wildcard, LIST* list) {
    if(glob_type == GLOB_FS) {
        glob_t result;
        int g_res;
        if((g_res = glob(wildcard, GLOB_NOSORT|GLOB_NOESCAPE|GLOB_NOCHECK, 0, &result)) != 0) {
            errno = g_res;
            return 0;
        }
        char* absolute_name;
        for(int i = result.gl_offs; result.gl_pathv[i]; ++i) {
            // retrieve absolute name of each file
            absolute_name = absolute_path(result.gl_pathv[i]);
            NODE* node = xmalloc(sizeof(NODE));
            node->str = xmalloc(strlen(absolute_name)+1);
            strcpy(node->str, absolute_name);
            // insert filenames into a list (best effort: no error check here)
            list_insert_unique(list, node);
        }
        globfree(&result);
    }
    else {
        LIST* temp_list = (LIST*) xzalloc(sizeof(LIST));
        temp_list->first = (NODE*) xzalloc(sizeof(NODE));
        // retrieve all tags
        if( !type_retrieve_list(elem_type, temp_list)) {
           return 0;
        }
        // keep only elements having name matching wildcard
        NODE* ptr = temp_list->first;
        while(ptr->next) {
            if(fnmatch(wildcard, ptr->next->str, FNM_NOESCAPE) == FNM_NOMATCH) {
                // remove non-matching elem
                NODE* temp = ptr->next;
                ptr->next = temp->next;
                free(temp->str);
                free(temp);			
                --temp_list->count;                
            }
            else ptr = ptr->next;
            if(!ptr) break;            
        }
        // store result into target list
        list_merge(list, temp_list);      
        list_free(temp_list);
    }
    return 1;
}

/*
type specifies the type of elements pointed by elems list
*/
int list_retrieve_list(int type, LIST* elems, LIST* list) {    
    NODE* ptr = elems->first;
    while(ptr->next) {
        // retrieve files related to current tag
        ELEM elem_related;
        int res = elem_init(type, ptr->next->str, &elem_related, 0);
        if( res <= 0) {
            // error : non-existing tag or reading error
            return 0;
        }                        
        // add list related to current elem to resulting list
        if(elem_retrieve_list(&elem_related, list) < 0) {
            // error while retrieving list from file
            return 0;                        
        }
        ptr = ptr->next;
    }
    return 1;
}
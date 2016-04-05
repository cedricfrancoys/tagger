/* tagger.c - main driver file for tagger program.

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
*/


/* The purpose of this program is to allow applying tags on files and directories.
 It does so by maintaining consistency into a filesystem-database consisting of 
 two directories containing files discribing symetrical relations (many to many).

 For optimum environment compatibilty, working charset is UTF-8,
 while input and output charset (as well as pathnames syntax) are OS-dependant.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stddef.h>
#include <getopt.h>
#include <errno.h>
#include <stdarg.h>

#include "env.h"
#include "charset.h"
#include "hash.h"
#include "xalloc.h"
#include "elem.h"
#include "list.h"
#include "error.h"
#include "eval.h"
#include "tagger.h"

/* Global flags */

/* verbose flag
Possible values:
 0    quiet
 1    normal (default)
 2    debug
*/
int verbose_flag = 1;

/* mode flag
Defines what kind of element current operation must be applied on ('tags' or 'files').
Possible values:
 1    ELEM_TAG (default)
 2    ELEM_FILE
*/
int mode_flag = ELEM_TAG;

/* local DB flag
Allows to force using a local database (in which case, filenames stored in DB are relative to current folder).
Possible values:
 0    global database - user's home dir (default)
 1    local database - current directory
*/
int local_flag = 0;

/* trash flag
Allows to restrict current operation to trashed elements only.
Possible values:
 0    normal (default)
 1    trashed elements
*/
int trash_flag = 0;


/* ELEM_DIR is defined in env.c
 Array holding the names of the sub-directories for the database.
*/
extern const char* ELEM_DIR[];


/* Available options
*/
static struct option options[] = {
    {"quiet",   0,    &verbose_flag, 0},
    {"debug",   0,    &verbose_flag, 2},
    {"files",   0,    &mode_flag, ELEM_FILE},
    {"tags",    0,    &mode_flag, ELEM_TAG},
    {"local",   0,    &local_flag, 1},
    {"trash",   0,    &trash_flag, 1},
    {"help",    0,    0, 'h'},
    {"version", 0,    0, 'v'},
    {0, 0, 0, 0}
};

/* Available operations and related handlers
*/
static struct operation operations[] = {
    {"init",    op_init},
    {"create",  op_create},
    {"clone",   op_clone},
    {"delete",  op_delete},
    {"recover", op_recover},
    {"rename",  op_rename},
    {"merge",   op_merge},
    {"tag",     op_tag},
    {"list",    op_list},    
    {"files",   op_files},
    {"tags",    op_tags},
    {"query",   op_query},    
    {0, 0}
};


/* Output information about current version.
*/
void version(void) {
    const char* VERSION = "tagger 1.0";
    const char* LICENSE = "\
Written by Cedric Francoys\n\
Copyright (C) 2015, Some Rights Reserved\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.";

    puts(VERSION);
    puts(LICENSE);
}

/* Display usage information.
 If status value is EXIT_SUCCESS, display the full help.
*/
void usage (int status) {
    if (status != 0) {
        puts("USAGE: tagger [OPTION] OPERATION [PARAMETERS]");
        puts("Try 'tagger --help' for more information.");
    }
    else {
        puts("USAGE: tagger [OPTION] OPERATION [PARAMETERS]");
        puts("OPTIONS:\n\
  --tags        (default) Set mode to apply operation on 'tag' elements\n\
  --files       Set mode to apply operation on 'file' elements\n\
  --local       Force using a local database (current folder)\n\
  --trash       Restrict current operation to trashed elements only\n\
  --quiet       Suppress all normal output\n\
  --debug       Output program trace and internal errors\n\
  --help        Display this help text\n\
  --version     Display version information"
        );
        puts("OPERATIONS:\n\
  init          Setup tagger environment (create an empty database)\n\
  create        Create one or more new tag(s)\n\
  clone         Create a new element by copying all relations from another\n\
  delete        Delete one or more element(s) (all relations will be lost)\n\
  recover       Recover a previously deleted element\n\
  rename        Rename an element\n\
  merge         Merge two elements (existing relations will be applied to both)\n\
  tag           Add(+) or remove(-) tag(s) to/from one or more files\n\
  list          Show all elements in database for specified mode\n\
  query         Retrieve all elements matching given criteria (depends on mode)\n\
  tags          Shorthand for \"tagger --tags list\"\n\
  files         Shorthand for \"tagger --files list\""
        );
        puts("Examples:\n\
  tagger create mp3 music\n\
  tagger tag +mp3 +music sound.mp3\n\
  tagger -music sound.mp3\n\
  tagger merge mp3 music\n\
  tagger query sound.mp3\n\
  tagger --files list"
        );
    }
}

void op_init(int argc, char* argv[], int index) {
    // check for application environment
    if(!check_env()) {
        if(!setup_env()) {
            raise_error(ERROR_ENV, "Unable to set up environment");
        }
        trace(TRACE_NORMAL, "Environment successfully created.");
    }
    else {
        // environment already inbitialized
        trace(TRACE_NORMAL, "Environment already set up: nothing to do.");
    }
}

/* Create one or more tags.
 Already existing tags are ignored.
 Output the numbers of created tags and ignored tags.
 On error, displays a message and exits.
*/
void op_create(int argc, char* argv[], int index){
    // we should have received a list of names
    if( index >= argc) {
        usage(1);
        raise_error(ERROR_USAGE, "Wrong number of arguments.");
    }
    if( mode_flag != ELEM_TAG) {
        usage(1);
        raise_error(ERROR_USAGE, "Operation 'create' applies only on tag elements.");
    }
    int n = 0, m = 0;
    for(int i = index; i < argc; ++i) {
        trace(TRACE_DEBUG, "creating tag '%s' : ", argv[i]);
        ELEM elem;
        int res = elem_init(ELEM_TAG, argv[i], &elem, 1);
        if(res <= 0) {
            raise_error(ERROR_ENV,
                        "%s:%d - Unexpected error occured when creating file %s for tag",
                        __FILE__, __LINE__, elem.file, elem.name);
        }
        else if(res == 2) {
            // tag was successfuly created
            ++n;
            trace(TRACE_DEBUG, "OK\n");
        }
        else {
            // tag already exists
            ++m;
            trace(TRACE_DEBUG, "Tag '%s' already exists\n", argv[i]);
        }
    }
    trace(TRACE_NORMAL, "%d tag(s) successfully created, %d tag(s) ignored.", n, m);
}

/* Create a new element that contains all relations of an existing element.
 example: tagger clone mp3 music
 where mp3 is an existing tag and music does not exist yet.
*/
void op_clone(int argc, char* argv[], int index){
    // we expect exactly two tags
    if( (argc-index) != 2) {
        usage(1);
        raise_error(ERROR_USAGE, "Wrong number of arguments.");
    }
    
    char* elem1_name = argv[index];
    char* elem2_name = argv[index+1];
    if( mode_flag == ELEM_FILE) {
        // ensure that the destination file exists in the file system
        elem2_name = absolute_path(elem2_name);
        FILE* fp = fopen(elem2_name, "r");
        if(!fp) {
            raise_error(ERROR_USAGE, "Operation 'clone' cannot be applied on non-existing file '%s'.", elem2_name);
        }
        fclose(fp);
    }

    // create a new element
    ELEM elem1, elem2;
    int res = elem_init(mode_flag, elem2_name, &elem2, 1);
    if( res <= 0) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when creating file '%s' for %s '%s'",
                    __FILE__, __LINE__, elem2.file, (mode_flag==ELEM_TAG)?"tag":"file", elem2.name);
    }
    if( res != 2){
        // return value must be 2 (created)
        raise_error(ERROR_USAGE, "A %s named '%s' already exists.", (mode_flag==ELEM_TAG)?"tag":"file", elem2.name);
    }
    if( elem_init(mode_flag, elem1_name, &elem1, 0) <= 0 ){
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when retrieving %s '%s'",
                    __FILE__, __LINE__, (mode_flag==ELEM_TAG)?"tag":"file", elem1.name);
    }
    // merge both elements
    op_merge(argc, argv, index);
}

/* Destroy one or more element(s).
 Any existing relations are removed as well.
*/
void op_delete(int argc, char* argv[], int index){
    int elems_i = 0, err_i = 0;
	LIST* list = (LIST*) xzalloc(sizeof(LIST));
	list->first = (NODE*) xzalloc(sizeof(NODE));

    // first pass : build a list with all elements to be deleted
    for(int i = index; i < argc; ++i) {
        if(strchr(argv[i], '*') != NULL) {
            // given name contains wildcard : handle with globbing
            glob_retrieve_list(GLOB_DB, mode_flag, argv[i], list);
        }
        else {
            ELEM elem;
            if(elem_init(mode_flag, argv[i], &elem, 0) <= 0) {
                raise_error(ERROR_RECOVERABLE, "%s '%s' not found", (mode_flag==ELEM_TAG)?"Tag":"File", argv[i]);
                continue;
            }
            NODE* node = xmalloc(sizeof(NODE));
            node->str = xmalloc(strlen(argv[i])+1);
            strcpy(node->str, argv[i]);
            list_insert_unique(list, node);
        }
    }
    // second pass : remove all elements in the list
    NODE* ptr = list->first;
    while(ptr->next) {
        ELEM elem;
        if(elem_init(mode_flag, ptr->next->str, &elem, 0) <= 0) {
            raise_error(ERROR_RECOVERABLE, "%s '%s' not found", (mode_flag==ELEM_TAG)?"Tag":"File", ptr->next->str);
            ++err_i;
            continue;
		}
		else ++elems_i;
        // open the element's file
        FILE* fp = fopen(elem.file, "r");
        char elem_name[ELEM_NAME_MAX];
        // read/skip first line
        if(fgets(elem_name, ELEM_NAME_MAX, fp) == NULL) {
            // unable to read from file
            raise_error(ERROR_ENV,
                        "%s:%d - Couldn't open '%s' for reading",
                        __FILE__, __LINE__, elem.file);
        }
        else {
            // for each pointed file element
            while(fgets(elem_name, ELEM_NAME_MAX, fp)) {
                // remove the newline char
                elem_name[strlen(elem_name)-1] = 0;
                ELEM el_related;
                elem_init((mode_flag%2)+1, elem_name+1, &el_related, 0);
                // remove relation with the element being deleted
                elem_relate(ELEM_REM, &el_related, &elem);
            }
        }
        fclose(fp);
        // deleted element's file
        //if(unlink(elem.file) < 0) {
        // instead of unlinking, we rename the file by appending a ".trash" to it                
        char newname[ELEM_NAME_MAX];
        sprintf(newname, "%s.trash", elem.file);
        if(rename(elem.file, newname) < 0) {
            // unable to delete file
            raise_error(ERROR_ENV,
                        "%s:%d - Couldn't delete file '%s'",
                        __FILE__, __LINE__, elem.file);
        }
        ptr = ptr->next;
    }
    list_free(list);
    trace(TRACE_NORMAL, "%d %s(s) successfuly deleted, %d %s(s) ignored.", elems_i, (mode_flag==ELEM_TAG)?"tag":"file", err_i, (mode_flag==ELEM_TAG)?"tag":"file");
}

/* Recover previously deleted element(s).
 Any formerly existing relations are recovered as well.
*/
void op_recover(int argc, char* argv[], int index){
    int elems_i = 0, err_i = 0;
	LIST* list = (LIST*) xzalloc(sizeof(LIST));
	list->first = (NODE*) xzalloc(sizeof(NODE));

    // first pass : build a list with all elements to be recovered
    for(int i = index; i < argc; ++i) {
        if(strchr(argv[i], '*') != NULL) {
            // given name contains wildcard : handle with globbing
            glob_retrieve_list(GLOB_DB, mode_flag, argv[i], list);
        }
        else {            
            NODE* node = xmalloc(sizeof(NODE));
            node->str = xmalloc(strlen(argv[i])+1);
            strcpy(node->str, argv[i]);
            list_insert_unique(list, node);
        }
    }
    // second pass : try to restore all elements in the list
    NODE* ptr = list->first;
    while(ptr->next) {    
        char* elem_name = ptr->next->str;        
        char* elem_file = resolve_name(mode_flag, elem_name);
        char elem_trash[ELEM_NAME_MAX];        
        sprintf(elem_trash, "%s.trash", elem_file);
        if(check_file(elem_name, elem_trash) <= 0){
            // unable to find trash file
            raise_error(ERROR_RECOVERABLE, "File '%s' not found", elem_trash);                       
            trace(TRACE_DEBUG,
                        "%s:%d - Couldn't locate file '%s' for %s '%s'",
                        __FILE__, __LINE__, elem_trash, (mode_flag==ELEM_TAG)?"tag":"file", elem_name);
                        
            ++err_i;
        }
        else {
            // restore file
            if(rename(elem_trash, elem_file) < 0) {
                // unable to restore file
                raise_error(ERROR_RECOVERABLE, "Unable to rename '%s' to '%s'", elem_trash, elem_file);
                trace(TRACE_DEBUG,
                            "%s:%d - Couldn't restore file '%s'",
                            __FILE__, __LINE__, elem_file);
                ++err_i;
            }
            else {                    
                ELEM elem;
                elem_init(mode_flag, elem_name, &elem, 0);
                // open the element's file
                FILE* fp = fopen(elem.file, "r");
                if(!fp) {
                    raise_error(ERROR_RECOVERABLE, "File '%s' not found", elem.file);
                    trace(TRACE_DEBUG,
                                "%s:%d - Couldn't open '%s' for reading",
                                __FILE__, __LINE__, elem.file);                
                }
                else {
                    char line[ELEM_NAME_MAX];
                    // read/skip first line
                    if(fgets(line, ELEM_NAME_MAX, fp) == NULL) {
                        // unable to read from file
                        raise_error(ERROR_RECOVERABLE, "Error reading file '%s'", elem.file);
                        trace(TRACE_DEBUG,
                                    "%s:%d - Couldn't read from '%s'",
                                    __FILE__, __LINE__, elem.file);
                        ++err_i;                                
                    }
                    else {
                        // for each pointed element
                        while(fgets(line, ELEM_NAME_MAX, fp)) {
                            // remove the newline char
                            line[strlen(line)-1] = 0;
                            ELEM el_related;
                            elem_init((mode_flag%2)+1, line+1, &el_related, 0);
                            // remove relation with the element being deleted
                            elem_relate(ELEM_ADD, &el_related, &elem);
                        }
                        ++elems_i;
                    }
                    fclose(fp);                                    
                }
            }            
        }
        free(elem_file);
        ptr = ptr->next;
    }
    list_free(list);
    trace(TRACE_NORMAL, "%d %s(s) successfuly recovered, %d %s(s) ignored.", elems_i, (mode_flag==ELEM_TAG)?"tag":"file", err_i, (mode_flag==ELEM_TAG)?"tag":"file");    
}

/* Merge two elements.
 Relations from each element are added to the other.
*/
void op_merge(int argc, char* argv[], int index){
    int elems_i = 0;
    // we should have received a list of two elements or more
    if( (index+1) >= argc) trace(TRACE_NORMAL, "Nothing to do.");
    else {
    	LIST* list = (LIST*) xzalloc(sizeof(LIST));
    	list->first = (NODE*) xzalloc(sizeof(NODE));    
        // create an array of files from all given elements
        for(int i = index; i < argc; ++i) {
            ELEM elem;
            elem_init(mode_flag, argv[i], &elem, 0);
            if(elem_retrieve_list(&elem, list) < 0) {
                raise_error(ERROR_ENV,
							"%s:%d - Unexpected error occured while retrieving list from file %s",
							__FILE__, __LINE__, elem.file);
            }
        }
        // update relations with resulting array
        for(int i = index; i < argc; ++i) {
            ELEM elem;
            elem_init(mode_flag, argv[i], &elem, 0);
            // (re)add current element to each element in the list
            for(NODE* node = list->first->next; node; node = node->next) {
                ELEM el_related;
                elem_init((mode_flag%2)+1, node->str, &el_related, 0);
                if( elem_relate(ELEM_ADD, &el_related, &elem) < 0 ) {
                    raise_error(ERROR_ENV,
								"%s:%d - Unexpected error while adding tag %s to file %s",
								__FILE__, __LINE__, elem.name, el_related.name);
                }
            }
            ++elems_i;
        }
        list_free(list);
        trace(TRACE_NORMAL, "%d %s successfuly merged.", elems_i, (mode_flag==ELEM_TAG)?"tags":"files");    
    }
}

/* Change the name of specified element to given name.
*/
void op_rename(int argc, char* argv[], int index){
    // we expect exactly two tags
    if( (argc-index) != 2) {
        usage(1);
        raise_error(ERROR_USAGE, "Wrong number of arguments.");
    }
    // create a new elem
    ELEM elem1, elem2;
    int res = elem_init(mode_flag, argv[index+1], &elem2, 1);
    if( res <= 0) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when creating file '%s' for element '%s'",
                    __FILE__, __LINE__, elem2.file, elem2.name);
    }
    if( res != 2){
        // return value must be 2 (created)
        raise_error(ERROR_USAGE, "%s '%s' already exists.", (mode_flag==ELEM_TAG)?"tag":"file", elem2.name);
    }
    
    // trace(TRACE_NORMAL, "1 %s successfuly created.", (mode_flag==ELEM_TAG)?"tag":"file");
    
    if( elem_init(mode_flag, argv[index], &elem1, 0) <= 0 ){
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when retrieving element %s",
                    __FILE__, __LINE__, elem1.name);
    }
    
    int temp_flag = verbose_flag;

    trace(TRACE_DEBUG, "merging %s '%s' and '%s'", (mode_flag==ELEM_TAG)?"tag":"file", elem1.name, elem2.name);    
    // merge both elements
    verbose_flag = 0;
        op_merge(argc, argv, index);
    verbose_flag = temp_flag;

    // delete original element    
    trace(TRACE_DEBUG, "deleting %s '%s'", (mode_flag==ELEM_TAG)?"tag":"file", elem1.name);
    verbose_flag = 0;
        op_delete(argc-1, argv, index);
    verbose_flag = temp_flag;
    
    trace(TRACE_NORMAL, "1 %s successfuly renamed.", (mode_flag==ELEM_TAG)?"tag":"file");
}

/* Add one or more tag(s) to onbe or more file(s).
*/
void op_tag(int argc, char* argv[], int index){
    char* add_tags[argc];
    char* rem_tags[argc];
    char* files[argc];
    int add_i = 0, rem_i = 0, files_i = 0, tags_i = 0;

    // 1) process arguments : memorize actions and targeted files
    for(int i = index; i < argc; ++i) {
        if(argv[i][0] == '+') {
            add_tags[add_i] = argv[i]+1;
            ++add_i;
        }
        else if(argv[i][0] == '-') {
            rem_tags[rem_i] = argv[i]+1;
            ++rem_i;
        }
        else {
            files[files_i] = argv[i];
            ++files_i;
        }
    }

    // 2) apply changes to each file    
    for(int i = 0; i < files_i; ++i) {
        ELEM el_file;
        if( elem_init(ELEM_FILE, files[i], &el_file, 1) <= 0 ) {
            raise_error(ERROR_RECOVERABLE,
            			"%s:%d - Unexpected error occured when creating file '%s' for file '%s'",
            			__FILE__, __LINE__, el_file.file, el_file.name);
            continue;
        }
        for(int j = 0; j < add_i; ++j) {
            ELEM el_tag;
            // create tag if it does not exist yet
            int res = 0;
            if( (res = elem_init(ELEM_TAG, add_tags[j], &el_tag, 1)) <= 0 ) {
		        raise_error(ERROR_ENV,
		        			"%s:%d - Unexpected error occured when creating file '%s' for tag '%s'",
		        			__FILE__, __LINE__, el_tag.file, el_tag.name);

            }
            else if(res == 2) tags_i++;
            // add tag to file elem & file to tag elem
            if( elem_relate(ELEM_ADD, &el_file, &el_tag) < 0 ) {
                raise_error(ERROR_ENV,
                            "%s:%d - Unexpected error while adding tag '%s' to file '%s'",
                            __FILE__, __LINE__, el_tag.name, el_file.name);
            }
        }
        for(int j = 0; j < rem_i; ++j) {
            ELEM el_tag;
            // if tag exists, remove it from file
            if(elem_init(ELEM_TAG, rem_tags[j], &el_tag, 0) > 0) {
                // remove tag from file elem & file from tag elem
                if( elem_relate(ELEM_REM, &el_file, &el_tag) < 0 ) {
                    raise_error(ERROR_ENV,
                                "%s:%d - Unexpected error while removing tag '%s' from file '%s'",
                                __FILE__, __LINE__, el_tag.name, el_file.name);
                }
            }
        }
    }
    if(files_i <= 0) trace(TRACE_NORMAL, "Nothing to do.");
    else {
        if(tags_i > 0)	trace(TRACE_NORMAL, "%d tag(s) created.", tags_i);
        if(add_i > 0)	trace(TRACE_NORMAL, "%d tag(s) added to %d files(s).", add_i, files_i);
        if(rem_i > 0)	trace(TRACE_NORMAL, "%d tag(s) removed from %d files(s).", rem_i, files_i);
    }
}

void op_list(int argc, char* argv[], int index) {
	LIST* list = (LIST*) xzalloc(sizeof(LIST));
	list->first = (NODE*) xzalloc(sizeof(NODE));
    
    // argument may be used as mask for limiting resulting list (ex. tagger --files list "C:\test\*")
    // this allows to check a single element or to retrieve all nodes inside a given directory
    if(index < argc) {
        if(strchr(argv[index], '*') != NULL) {
            // given name contains wildcard : handle with globbing
            if(!glob_retrieve_list(GLOB_DB, mode_flag, argv[index], list)) {
                raise_error(ERROR_ENV,
                            "%s:%d - Unable to retrieve %s list for pattern '%s'",
                            __FILE__, __LINE__, (mode_flag==ELEM_TAG)?"files":"tags", argv[index]);            
            }
        }
        else {
            // check if given element is present in DB
            ELEM elem;
            if( elem_init(mode_flag, argv[index], &elem, 0) > 0) {
                NODE* node = xmalloc(sizeof(NODE));
                node->str = xmalloc(strlen(elem.name)+1);
                strcpy(node->str, elem.name);                
                list_insert_unique(list, node);                
            }
        }
    }
    else {
        trace(TRACE_DEBUG, "reading %s directory", (mode_flag==ELEM_TAG)?"tags":"files");
        if( !type_retrieve_list(mode_flag, list)) {
            raise_error(ERROR_ENV,
                        "%s:%d - Couldn't open %s directory",
                        __FILE__, __LINE__, (mode_flag==ELEM_TAG)?"tags":"files");
        }
    }
    // output resulting list
    if(!list->count) {
        if(index < argc) {
            trace(TRACE_NORMAL, "No %s with given name in database.", (mode_flag==ELEM_TAG)?"tag":"file"); 
        }
        else {
            if(mode_flag==ELEM_TAG) trace(TRACE_NORMAL, "No tag in database.");
            else                    trace(TRACE_NORMAL, "No file has been tagged yet.");
        }
    }
    else if(!list_output(list)) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unable to output %s list"
                    __FILE__, __LINE__, (mode_flag==ELEM_TAG)?"tags":"files");
    }
    list_free(list);
}

/* Retrieve all files currently tagged.
*/
void op_files(int argc, char* argv[], int index){
    mode_flag = ELEM_FILE;
    op_list(argc, argv, index);
}

/* Show all existing tags.
*/
void op_tags(int argc, char* argv[], int index){
    mode_flag = ELEM_TAG;
    op_list(argc, argv, index);
}

/* Retrieve all files matching given criteria.
Cretaria consist of a list of elements or a query pointing to elements, that are related to the elements we're looking for.

Arguments might be either a simple string (element name or wildcard, ex.: mp3, music/* or C:\test*),
or a search query (ex.: "mp3 & !music/soundtracks")
*/
void op_query(int argc, char* argv[], int index) {
    if(index >= argc) {
        op_list(argc, argv, index);
    }
    else {
        // from now on we should have received criteria (list of tags names)

        // obtain the files directory
        char* install_dir = get_install_dir();
        // allocate path (adding an extra char for slash/separator)
        char* elems_dir = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[mode_flag])+2);
        sprintf(elems_dir, "%s/%s", install_dir, ELEM_DIR[mode_flag]);

        LIST* list_elems = (LIST*) xzalloc(sizeof(LIST));
        list_elems->first = (NODE*) xzalloc(sizeof(NODE));

		// use arguments to build resulting list
        for(int i = index; i < argc; ++i) {
			// each argument should be either a tagname or a query
			// in any case, those arguments are processed in sequence to build a disjunction (OR clauses)

			// detect if argument has to be processed as tagname or as a query
// disabling query syntax when given arg is a filename (filenames can be complex and building queries with them is of little use)
			if( mode_flag == ELEM_TAG || !is_query(argv[i]) ) {
                if(strchr(argv[i], '*') != NULL) {
                    // given name contains wildcard : handle with globbing
                    LIST* list_related = (LIST*) xzalloc(sizeof(LIST));
                    list_related->first = (NODE*) xzalloc(sizeof(NODE));
                    // retrieve all elements pointed by wildcard 
                    // (we force DB globbing instead of FS globbing by using type ELEM_TAG)
                    if(!glob_retrieve_list(GLOB_DB, (mode_flag%2)+1, argv[i], list_related)) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unable to retrieve %s list for pattern '%s'",
                                    __FILE__, __LINE__, (mode_flag==ELEM_TAG)?"files":"tags", argv[i]);
                    }
                    // add all files related to retrieved list to resulting list
                    if(!list_retrieve_list((mode_flag%2)+1, list_related, list_elems)) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unable to retrieve files list for pattern '%s'",
                                    __FILE__, __LINE__, argv[i]);
                    }
                    list_free(list_related);
                }
                else {
                    // process as a single elem name
                    ELEM elem;
                    int res = elem_init((mode_flag%2)+1, argv[i], &elem, 0);
                    if( res < 0) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unexpected error occured while looking for element '%s'",
                                    __FILE__, __LINE__, argv[i]);
                    }
                    else if(res) {
                        // add files tagged with current tag to resulting list
                        if(elem_retrieve_list(&elem, list_elems) < 0) {
                            raise_error(ERROR_ENV,
                                        "%s:%d - Unexpected error occured while retrieving list from file '%s'",
                                        __FILE__, __LINE__, elem.file);
                        }
                    }
                }
			}
			else {
				trace(TRACE_DEBUG, "query detected");
				// process as a query
                LIST* list_query = eval(argv[i]);
                if(!list_query) {
					raise_error(ERROR_ENV,
								"%s:%d - Unexpected error occured while interpreting query '%s'",
								__FILE__, __LINE__, argv[i]);
                }
                list_merge(list_elems, list_query);
                list_free(list_query);
			}
        }
        // output resulting files list
        if(!list_elems->count) {        
            if(mode_flag==ELEM_TAG) trace(TRACE_NORMAL, "No tag currently applied on given file(s).");
            else                    trace(TRACE_NORMAL, "No file currently tagged with given tag(s).");
        }
        else if(!list_output(list_elems)) {
            raise_error(ERROR_ENV,
                        "%s:%d - Unable to output elements list"
                        __FILE__, __LINE__);
        }

        list_free(list_elems);
        free(elems_dir);        
    }
}

int main(int argc, char* argv[]) {

    // no argument received: display usage and quit
    if (argc < 2) {
        usage(1);
        raise_error(ERROR_USAGE, "No argument received.");
    }

    // As we are going to work exclusively using UTF-8 charset,
    // we convert the entire argv to UTF-8
    char* cs_from = get_input_charset();
    for(int i = 1; i < argc; ++i) {
        if(strlen(argv[i]) == 0){
            // invalid argument
            usage(1);            
            raise_error(ERROR_USAGE, "Empty argument detected.");
        }
        // if something goes wrong, pointer is assigned to NULL
        argv[i] = strtoutf8(cs_from, argv[i]);
    }

    // process given arguments
    int opt, arg_i;

    // 1) check options
    for(arg_i = 1; arg_i < argc; ++arg_i) {
        opt = -1;
        // try to find matching option, if any
        if(argv[arg_i][0] != '-' || argv[arg_i][1] != '-') {
            // current argument does not match options syntax
            break;
        }
        for(int j = 0; options[j].name; ++j) {
            if(strcmp(argv[arg_i]+2, options[j].name) == 0) {
                opt = j;
                // process invoked option
                if(options[j].flag != NULL) {
                    *(options[j].flag) = options[j].val;
                }
                else {
                    switch(options[j].val) {
                        case 'h':
                            // display help
                            usage(0);
                            exit(0);
                            break;
                        case 'v':
                            // display app version
                            version();
                            exit(0);
                            break;
                    }
                }
            }
        }
        if(opt == -1) {
            // unknown option
            usage(1);
            raise_error(ERROR_USAGE, "Unknown option.");
        }
    }

    // 2 bis) check for environment
    if(!check_env()) {
        trace(TRACE_NORMAL, "Installation directory not found or corrupted... Try 'tagger init'");
    }
    
    // 2) check operation
    trace(TRACE_DEBUG, "checking operations");

    // handle non-option argv remaining arguments (first one should be an operation)
    if (arg_i < argc) {
        // try to find matching operation
        for(int i = 0; operations[i].name; ++i) {
            if(strcmp(argv[arg_i], operations[i].name) == 0) {
                trace(TRACE_DEBUG, "found matching operation: '%s'", operations[i].name);
                // dispatch actions and arguments processing to invoked operation
                operations[i].f(argc, argv, arg_i+1);
                return 0;
            }
        }
        // either omitted 'tag' operation or unknown operation
        // if next arg starts with a + or -, relay to 'tag' op.
        if(argv[arg_i][0] == '+' || argv[arg_i][0] == '-') {
            trace(TRACE_DEBUG, "assuming shorthand syntax for operation 'tag'");
            op_tag(argc, argv, arg_i);
        }
        else {
            usage(1);
            raise_error(ERROR_USAGE, "Invalid operation.");
        }
    }
    else {
        // no argument received: display usage and quit
        usage(1);
        raise_error(ERROR_USAGE, "No argument received.");
    }
    return EXIT_SUCCESS;
}

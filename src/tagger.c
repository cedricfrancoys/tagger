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


/* This application allows to apply tags on files and directories
 by maintaining consistency into a filesystem-database.
 That database consists of two directories containing files
 discribing symetrical relations (many to many).

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



/* Global verbose flag
Possible values are:
 0    quiet
 1    normal (default)
 2    debug
*/
int verbose_flag = 1;

/* Global mode flag
This var tells if current operation must be applied on 'tags' or 'files' elements.
Possible values are:
 1    ELEM_TAG (default)
 2    ELEM_FILE
*/
int mode_flag = ELEM_TAG;

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
    {"tags",    0,    &mode_flag, ELEM_FILE},
    {"help",    0,    0, 'h'},
    {"version", 0,    0, 'v'},
    {0, 0, 0, 0}
};

/* Available operations and related handlers
*/
static struct operation operations[] = {
    {"create",  op_create},
    {"clone",   op_clone},
    {"delete",  op_delete},
    {"recover", op_recover},
    {"rename",  op_rename},
    {"merge",   op_merge},
    {"tag",     op_tag},
    {"files",   op_files},
    {"tags",    op_tags},
    {0, 0}
};


/* Output information about current version.
*/
void version(void) {
    static const char* VERSION = "\
tagger 1.0\n\
\n\
Written by Cedric Francoys\n\
Copyright (C) 2015, Some Rights Reserved\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.";
    puts(VERSION);
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
  --quiet       Suppress all normal output\n\
  --debug       Output program trace and internal errors\n\
  --help        Display this help text and exit\n\
  --version     Display version information and exit"
        );
        puts("OPERATIONS:\n\
  create        Create one or more new tags\n\
  clone         Create a new tag by copying all existin relations from another\n\
  delete        Delete one or more tags (all relations will be lost)\n\
  rename        Rename an existing tag\n\
  merge         Merge two tags (relations of each tag will be applied to both)\n\
  tag           Add(+) or remove(-) tag(s) to/from one or more files\n\
  files         Show files matching the given criterias\n\
  tags          Show tags related to one or more files (no file means all tags)"
        );
        puts("Examples:\n\
  tagger create mp3 music\n\
  tagger tag +mp3 +music sound.mp3\n\
  tagger -music sound.mp3\n\
  tagger merge mp3 music\n\
  tagger tags sound.mp3"
        );
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

/* Create a new tag that contains all relations of a given tag.
 example: tagger clone mp3 music
 where mp3 is an existing tag and music does not exist yet
*/
void op_clone(int argc, char* argv[], int index){
    // we expect exactly two tags
    if( (argc-index) != 2) {
        usage(1);
        raise_error(ERROR_USAGE, "Wrong number of arguments.");
    }
    if( mode_flag != ELEM_TAG) {
        usage(1);
        raise_error(ERROR_USAGE, "Operation 'clone' applies only on tag elements.");
    }

    // create a new tag
    ELEM el_tag1, el_tag2;
    int res = elem_init(ELEM_TAG, argv[index+1], &el_tag2, 1);
    if( res <= 0) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when creating file '%s' for tag '%s'",
                    __FILE__, __LINE__, el_tag2.file, el_tag2.name);
    }
    if( res != 2){
        // return value must be 2 (created)
        raise_error(ERROR_USAGE, "A tag by target name '%s' already exists.", el_tag2.name);
    }
    if( elem_init(ELEM_TAG, argv[index], &el_tag1, 0) <= 0 ){
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when retrieving tag '%s'",
                    __FILE__, __LINE__, el_tag1.name);
    }
    // merge both tags
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
            glob_retrieve_list(mode_flag, argv[i], list);
        }
        else {
            ELEM elem;
            if(elem_init(mode_flag, argv[i], &elem, 0) <= 0) {
                raise_error(ERROR_RECOVERABLE, "Element '%s' not found", argv[i]);
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
            raise_error(ERROR_RECOVERABLE, "Tag '%s' not found", ptr->next->str);
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
// instead of unlinking, we should rename the file by apending a .trash to it        
        if(unlink(elem.file) < 0) {
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
	LIST* list = (LIST*) xzalloc(sizeof(LIST));
	list->first = (NODE*) xzalloc(sizeof(NODE));

    // first pass : build a list with all elements to be deleted
    for(int i = index; i < argc; ++i) {
        if(strchr(argv[i], '*') != NULL) {
            // given name contains wildcard : handle with globbing
            glob_retrieve_list(mode_flag, argv[i], list);
        }
        else {
            ELEM elem;
            if(elem_init(mode_flag, argv[i], &elem, 0) <= 0) {
                raise_error(ERROR_RECOVERABLE, "Element '%s' not found", argv[i]);
                continue;
            }
            NODE* node = xmalloc(sizeof(NODE));
            node->str = xmalloc(strlen(argv[i])+1);
            strcpy(node->str, argv[i]);
            list_insert_unique(list, node);
        }
    }

    list_free(list);
}

/* Merge two elements.
 Relations from each element are added to the other.
*/
void op_merge(int argc, char* argv[], int index){
    // we should have received a list of two elements or more
    if( (index+1) >= argc) trace(TRACE_NORMAL, "Nothing to do.");
    else {
        NODE node = {0, 0};
        LIST list = {&node, 0};
        // create an array of files from all given elements
        for(int i = index; i < argc; ++i) {
            ELEM elem;
            elem_init(mode_flag, argv[i], &elem, 0);
            if(elem_retrieve_list(&elem, &list) < 0) {
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
            for(NODE* node = list.first->next; node; node = node->next) {
                ELEM el_related;
                elem_init((mode_flag%2)+1, node->str, &el_related, 0);
                if( elem_relate(ELEM_ADD, &el_related, &elem) < 0 ) {
                    raise_error(ERROR_ENV,
								"%s:%d - Unexpected error while adding tag %s to file %s",
								__FILE__, __LINE__, elem.name, el_related.name);
                }
            }
        }
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
    if( elem_init(mode_flag, argv[index], &elem1, 0) <= 0 ){
        raise_error(ERROR_ENV,
                    "%s:%d - Unexpected error occured when retrieving element %s",
                    __FILE__, __LINE__, elem1.name);
    }
    // merge both elements
    trace(TRACE_DEBUG, "merging %s '%s' and '%s'", (mode_flag==ELEM_TAG)?"tag":"file", elem1.name, elem2.name);
    op_merge(argc, argv, index);
    // delete original element
    trace(TRACE_DEBUG, "deleting %s '%s'", (mode_flag==ELEM_TAG)?"tag":"file", elem1.name);
    op_delete(argc-1, argv, index);
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
    char* absolute_name;
    for(int i = 0; i < files_i; ++i) {
        ELEM el_file;
        // retrieve absolute name of the file
        if( !(absolute_name = absolute_path(files[i])) ) {
            raise_error(ERROR_RECOVERABLE, "File '%s' not found", files[i]);
            continue;
        }
        if( elem_init(ELEM_FILE, absolute_name, &el_file, 1) <= 0 ) {
            raise_error(ERROR_ENV,
            			"%s:%d - Unexpected error occured when creating file '%s' for file '%s'",
            			__FILE__, __LINE__, el_file.file, el_file.name);
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

/* Retrieve all files matching given criteria.
 Arguments might be either a simple string (tagname or wildcard, ex.: mp3 or music/*),
or a search query (ex.: "mp3 & !music/soundtracks")
 (This is probabily one of the two most useful function for final user.)
*/
void op_files(int argc, char* argv[], int index){
    // obtain the files directory
    char* install_dir = get_install_dir();
    // allocate path (adding an extra char for slash/separator)
    char* files_dir = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[ELEM_FILE])+2);
    sprintf(files_dir, "%s/%s", install_dir, ELEM_DIR[ELEM_FILE]);

	LIST* list_files = (LIST*) xzalloc(sizeof(LIST));
	list_files->first = (NODE*) xzalloc(sizeof(NODE));

    if(argc > index) {
        // from now on we should have received criteria (list of tags names)

		// use arguments to build resulting list
        for(int i = index; i < argc; ++i) {
			// each argument should be either a tagname or a query
			// in any case, those arguments are processed in sequence to build a disjunction (OR clauses)

			// detect if argument has to be processed as tagname or as a query
			if( !is_query(argv[i]) ) {
                if(strchr(argv[i], '*') != NULL) {
                    // given name contains wildcard : handle with globbing
                    LIST* list_tags = (LIST*) xzalloc(sizeof(LIST));
                    list_tags->first = (NODE*) xzalloc(sizeof(NODE));
                    // retrieve all tags pointed by wildcard
                    if(!glob_retrieve_list(ELEM_TAG, argv[i], list_tags)) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unable to retrieve tags list for pattern '%s'",
                                    __FILE__, __LINE__, argv[i]);
                    }
                    // add all files related to tags list to resulting files list
                    if(!list_retrieve_list(ELEM_TAG, list_tags, list_files)) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unable to retrieve files list for pattern '%s'",
                                    __FILE__, __LINE__, argv[i]);
                    }
                    list_free(list_tags);
                }
                else {
                    // process as a single tag name
                    ELEM el_tag;
                    int res = elem_init(ELEM_TAG, argv[i], &el_tag, 0);
                    if( res < 0) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unexpected error occured while looking for tag '%s'",
                                    __FILE__, __LINE__, argv[i]);
                    }
                    else if(!res) {
                        raise_error(ERROR_USAGE, "Tag '%s' does not exist.", argv[i]);
                    }
                    // add files tagged with current tag to resulting list
                    if(elem_retrieve_list(&el_tag, list_files) < 0) {
                        raise_error(ERROR_ENV,
                                    "%s:%d - Unexpected error occured while retrieving list from file '%s'",
                                    __FILE__, __LINE__, el_tag.file);
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
                list_merge(list_files, list_query);
                list_free(list_query);
			}
        }
    }
    else {
        // no more argument : display all tagged files
        trace(TRACE_DEBUG, "reading files directory");
		if( !type_retrieve_list(ELEM_FILE, list_files)) {
            raise_error(ERROR_ENV,
            			"%s:%d - Couldn't open files directory",
            			__FILE__, __LINE__);
		}
    }
    // output resulting files list
    if(!list_files->count) {
        trace(TRACE_NORMAL, "No file matching given pattern.");
    }
    else if(!list_output(list_files)) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unable to output files list"
                    __FILE__, __LINE__);
    }
    list_free(list_files);
    free(files_dir);
}

/* Show all tags currently applied to specified file(s).
*/
void op_tags(int argc, char* argv[], int index){
    // obtain the tags directory
    char* install_dir = get_install_dir();
    // allocate path, adding an extra char for slash/separator
    char* tags_dir = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[ELEM_TAG])+2);
    sprintf(tags_dir, "%s/%s", install_dir, ELEM_DIR[ELEM_TAG]);

    NODE t_node = {0, 0};
    LIST list_tags = {&t_node, 0};

    if(argc > index) {
        // from now on we should have received a list of files
        // 1) create a list of files for which we want to retrieve applied tags
        NODE f_node = {0, 0};
        LIST list_files = {&f_node, 0};
        char* absolute_name;
        for(int i = index; i < argc; ++i) {
            if(strchr(argv[i], '*') != NULL) {
                // given name contains wildcard : handle with globbing
                if(!glob_retrieve_list(ELEM_FILE, argv[i], &list_files)) {
                    raise_error(ERROR_ENV,
                                "%s:%d - Unable to retrieve file list for pattern '%s'",
                                __FILE__, __LINE__, argv[i]);
                }
            }
            else {
                // we assume current arg is a single filename
                // add it to files list
                NODE* node = xmalloc(sizeof(node));
                // check if given file exists
                if( !(absolute_name = absolute_path(argv[i])) ) {
                    raise_error(ERROR_RECOVERABLE, "File '%s' not found", argv[i]);
                }
                else {
                    node->str = xmalloc(strlen(absolute_name)+1);
                    strcpy(node->str, absolute_name);
                    list_insert_unique(&list_files, node);
                }
            }
        }
        // 2) create a result list containing applied tags of all involved files (with no duplicates)
        for(NODE* node = list_files.first->next; node; node = node->next){
            ELEM el_file;
            if(elem_init(ELEM_FILE, node->str, &el_file, 0) > 0) {
                elem_retrieve_list(&el_file, &list_tags);
            }
        }
        list_free(&list_files);
    }
    else {
        // no more argument : display all existing tags
        trace(TRACE_DEBUG, "reading tags directory");
		if( !type_retrieve_list(ELEM_TAG, &list_tags)) {
            raise_error(ERROR_ENV,
            			"%s:%d - Couldn't open tags directory",
            			__FILE__, __LINE__);
		}
    }
    // output resulting tags names
    if(!list_tags.count) {
        trace(TRACE_NORMAL, "No tag currently applied on given file(s).");
    }
    else if(!list_output(&list_tags)) {
        raise_error(ERROR_ENV,
                    "%s:%d - Unable to output tags list"
                    __FILE__, __LINE__);

    }
    list_free(&list_tags);
    free(tags_dir);
}


int main(int argc, char* argv[]) {

    // check for application environment
    if(!check_env()) {
        trace(TRACE_NORMAL, "Installation directory not found or corrupted...");
        if(!setup_env()) {
            raise_error(ERROR_ENV, "Unable to set up environment");
        }
        trace(TRACE_NORMAL, "Environment successfully created.");
    }

    // no argument received: display usage and quit
    if (argc < 2) {
        usage(1);
        raise_error(ERROR_USAGE, "No argument received.");
    }

    // As we are going to work exclusively using UTF-8 charset,
    // we convert the entire argv to UTF-8
    char* cs_from = get_input_charset();
    for(int i = 1; i < argc; ++i) {
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

    return EXIT_SUCCESS;
}
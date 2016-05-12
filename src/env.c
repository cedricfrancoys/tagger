/* env.c - interface for checking and setting program environment.

    This file is part of the tagger program <http://www.github.com/cedricfrancoys/tagger>
    Copyright (C) Cedric Francoys, 2015, Yegen
    Some Right Reserved, GNU GPL 3 license <http://www.gnu.org/licenses/>
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>

#include "xalloc.h"
#include "elem.h"
#include "env.h"
#include "error.h"

/* use global setting vars, defined and set in main driver (tagger.c)
*/
extern char ENV_PATH[FILENAME_MAX];
extern char ENV_DIR[FILENAME_MAX];
extern char DB_NODE_SYNTAX[10];
extern char DB_CHARSET[32];


const char* ELEM_DIR[] = {"", "tags", "files"};

/* Define a constant holding an identifier of the current system.
*/
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
const char* OS_ENV = "WIN32";
const char* PATH_SEPARATOR = "\\";
const int   ROOT_LEN = 3;
#else
const char* OS_ENV = "POSIX";
const char* PATH_SEPARATOR = "/";
const int   ROOT_LEN = 1;
#endif


/* Convert a path string according to current environment.
 Deal with  backslashes ('\') and slashes ('/') conversions,
 and remove trailing slash(es) (according to POSIX standard).
*/
char* fix_path(char* path) {
    int i;
    for(i = 0; path[i] && i < FILENAME_MAX; ++i) {
        if(path[i] == '\\' || path[i] == '/') {
            path[i] = PATH_SEPARATOR[0];
        }
    }
    while(i && (path[i-1] == '/' || path[i-1] == '\\')) {
        path[--i] = '\0';
    }
    return path;
}

/* Obtain the absolute path of a given filename
*/
char* absolute_path(char* filename) {
    static char absolute_name[FILENAME_MAX];
    if(!realpath(filename, absolute_name)) {
        return NULL;
    }
    return fix_path(absolute_name);
}

/* Obtain the path of a file relatively to install dir
*/
char* relative_path(char* filename) {
    static char relative_name[FILENAME_MAX];
    char* absolute_name = absolute_path(filename);
    char reference_name[FILENAME_MAX];
// todo : use ENV_DIR as reference
    getcwd(reference_name, FILENAME_MAX);

    // init result string
    relative_name[0] = '\0';
    // check first char (under windows, if differs, we return absolute path)
    if(absolute_name[0] != reference_name[0]) {
        return absolute_name;
    }
    // make copies to avoid altering original strings
    char* path_a = strdup(absolute_name);
    char* path_r = strdup(reference_name);

    int inc;
    int size_a = strlen(path_a)+1;
    int size_r = strlen(path_r)+1;

    for(inc = 0; inc < size_a && inc < size_r; inc += strlen(path_a+inc)+1) {
        char* token_a = strchr(path_a+inc, PATH_SEPARATOR[0]);
        char* token_r = strchr(path_r+inc, PATH_SEPARATOR[0]);
        if(token_a) token_a[0] = '\0';
        if(token_r) token_r[0] = '\0';
        if(strcmp(path_a+inc, path_r+inc) != 0) break;
    }

    for(int inc_r = inc; inc_r < size_r; inc_r += strlen(path_r+inc_r)+1) {
        strcat(relative_name, "..");
        strcat(relative_name, PATH_SEPARATOR);
        if( !strchr(reference_name+inc_r, PATH_SEPARATOR[0]) ) break;
    }

    if(inc < size_a) strcat(relative_name, absolute_name+inc);
    return relative_name;
}

char* get_path(char* filename) {
    if(!strcmp(DB_NODE_SYNTAX, "relative")) return relative_path(filename);
    return absolute_path(filename);
}

/* Retrieve the installation directory (ex.:[user homedir]/.tagger)
*/
char* get_install_dir() {
    static char install_dir[FILENAME_MAX] = "";
    // compute install dir only once
    if(strlen(install_dir) == 0) {
        if(!strcasecmp(ENV_PATH, "home")) {
            if(strcmp(OS_ENV, "WIN32") == 0) {
                char* homedrive = getenv("HOMEDRIVE");
                char* homepath = getenv("HOMEPATH");
                // install_dir = xmalloc(strlen(homedrive)+strlen(homepath)+strlen(ENV_DIR)+2);
                sprintf(install_dir, "%s%s%s%s", homedrive, homepath, PATH_SEPARATOR, ENV_DIR);
            }
            else {
                char* home = getenv("HOME");
                // install_dir = xmalloc(strlen(home)+strlen(ENV_DIR)+2);
                sprintf(install_dir, "%s%s%s", home, PATH_SEPARATOR, ENV_DIR);
            }
        }
        else {
            if(!strcasecmp(ENV_PATH, "current")) {
                char current_dir[FILENAME_MAX];
                getcwd(current_dir, FILENAME_MAX);
                // if no DB is found in current or parent directories, current path will be returned
                // (it is the only case when this function returns a path that does not exist yet)
                sprintf(install_dir, "%s%s%s", current_dir, PATH_SEPARATOR, ENV_DIR);
                while(strlen(current_dir) > ROOT_LEN) {
                    char temp[FILENAME_MAX];
                    sprintf(temp, "%s%s%s", current_dir, PATH_SEPARATOR, ENV_DIR);
                    if(opendir(temp)) {
                        strcpy(install_dir, temp);
                        found = 1;
                        break;
                    }
                    char* sep = strrchr(current_dir, PATH_SEPARATOR[0]);
                    if(!sep) break;
                    *sep = 0;
                }
            }
            else sprintf(install_dir, "%s%s%s", ENV_PATH, PATH_SEPARATOR, ENV_DIR);
        }
    }
    return install_dir;
}


int check_env() {
    int result = 0;
    char* main_dir = get_install_dir();
    // allocate paths, adding an extra char for slash/separator
    char* files_dir = xmalloc(strlen(main_dir)+strlen(ELEM_DIR[ELEM_FILE])+2);
    char* tags_dir = xmalloc(strlen(main_dir)+strlen(ELEM_DIR[ELEM_TAG])+2);
    sprintf(files_dir, "%s%s%s", main_dir, PATH_SEPARATOR, ELEM_DIR[ELEM_FILE]);
    sprintf(tags_dir, "%s%s%s", main_dir, PATH_SEPARATOR, ELEM_DIR[ELEM_TAG]);

    if(opendir(main_dir) && opendir(files_dir) && opendir(tags_dir)) {
        result = 1;
    }

    free(files_dir);
    free(tags_dir);
    return result;
}

int setup_env() {
    char* install_dir = get_install_dir();
    // we add an extra char for slash/separator
    char* sub_dir = xmalloc(strlen(install_dir)+strlen(ELEM_DIR[ELEM_FILE])+2);

    DIR *dp;
    // create a '.tagger' directory if missing
    if(!(dp = opendir(install_dir))) {
        if(mkdir(install_dir, 0755) < 0) {
            return 0;
        }
    }
    // create sub directories if missing
    sprintf(sub_dir, "%s%s%s", install_dir, PATH_SEPARATOR, ELEM_DIR[ELEM_FILE]);
    if(!(dp = opendir(sub_dir))) {
        // create a '.tagger/files' directory
        if(mkdir(sub_dir, 0755) < 0) {
            return 0;
        }
    }
    sprintf(sub_dir, "%s%s%s", install_dir, PATH_SEPARATOR, ELEM_DIR[ELEM_TAG]);
    if(!(dp = opendir(sub_dir))) {
        // create a '.tagger/tags' directory
        if(mkdir(sub_dir, 0755) < 0) {
            return 0;
        }
    }
    free(sub_dir);
    return 1;
}

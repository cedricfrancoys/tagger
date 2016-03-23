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

#include "xalloc.h"
#include "elem.h"
#include "env.h"

/* local flag is defined and set in the main driver (tagger.c)
*/
extern int local_flag;


const char* APP_DIR = ".tagger";

const char* ELEM_DIR[] = {"", "tags", "files"};

/* Define a constant holding an identifier of the current system.
*/
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
const char* OS_ENV = "WIN32";
#else
const char* OS_ENV = "POSIX";
#endif  

/* We'll use a local-defined value for constant FILENAME_MAX.
 (which might not be defined on certain environement)
*/
#if defined( FILENAME_MAX )
#undef FILENAME_MAX
#endif

/* Force FILENAME_MAX macro to a local value for compatibility purpose.
 FILENAME_MAX serves as system limit for the length of a string holding a filename with full path.
 (Though 1024 chars long is a quite long path, this might be adjusted depending on user's specific needs.)
*/
#define FILENAME_MAX 1024

/* Convert a path string according to current environment.
 Deal with  backslashes ('\') and slashes ('/') conversions,
 and remove trailing slash(es) (according to POSIX standard).
*/
char* fix_path(char* path) {
    int i;
    char separator = '/';
    if(strcmp(OS_ENV, "WIN32") == 0) separator = '\\';
    for(i = 0; path[i] && i < FILENAME_MAX; ++i) {
        if(path[i] == '\\' || path[i] == '/') {
            path[i] = separator;
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
// todo 
    char* relative_path = absolute_path(filename);
    return NULL;
}

char* get_path(char* filename) {
    if(local_flag) return relative_path(filename);
    return absolute_path(filename);
}

/* Retrieve the installation directory ([user homedir]/.tagger)
*/
char* get_install_dir() {
    static char* install_dir = "";
    if(strlen(install_dir) == 0) {
        if(local_flag) {
            char current_dir[FILENAME_MAX];
            getcwd(current_dir, FILENAME_MAX);
            install_dir = xmalloc(strlen(current_dir)+strlen(APP_DIR)+2);
            getcwd(install_dir, FILENAME_MAX);
            sprintf(install_dir, "%s\\%s", current_dir, APP_DIR);           
        }
        else {
            if(strcmp(OS_ENV, "WIN32") == 0) {
                char* homedrive = getenv("HOMEDRIVE");
                char* homepath = getenv("HOMEPATH");
                install_dir = xmalloc(strlen(homedrive)+strlen(homepath)+strlen(APP_DIR)+2);
                sprintf(install_dir, "%s%s\\%s", homedrive, homepath, APP_DIR);           
            }
            else {
                char* home = getenv("HOME");
                install_dir = xmalloc(strlen(home)+strlen(APP_DIR)+2);
                sprintf(install_dir, "%s/%s", home, APP_DIR);
            }
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
    sprintf(files_dir, "%s/%s", main_dir, ELEM_DIR[ELEM_FILE]);
    sprintf(tags_dir, "%s/%s", main_dir, ELEM_DIR[ELEM_TAG]);  
    
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
    sprintf(sub_dir, "%s/%s", install_dir, ELEM_DIR[ELEM_FILE]);
    if(!(dp = opendir(sub_dir))) {
        // create a '.tagger/files' directory
        if(mkdir(sub_dir, 0755) < 0) {
            return 0;
        }
    }
    sprintf(sub_dir, "%s/%s", install_dir, ELEM_DIR[ELEM_TAG]);
    if(!(dp = opendir(sub_dir))) {
        // create a '.tagger/tags' directory
        if(mkdir(sub_dir, 0755) < 0) {
            return 0;
        }
    }
    free(sub_dir);
    return 1;
}
